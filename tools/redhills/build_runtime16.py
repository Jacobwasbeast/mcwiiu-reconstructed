"""Build the two-byte wchar_t C and C++ runtime the Wii U build links against.

The Cafe ABI's wchar_t is two bytes (the game is compiled with -fshort-wchar), but devkitPPC ships
newlib and libstdc++ built for a four-byte wchar_t, whose wide-character functions would read past every
string the game passes them. This builds both again with devkitPPC's GCC, from the sources and patches
devkitPro builds them from (tools/redhills/sources.py), with -fshort-wchar:

- newlib 4.6.0.20260123: wide-character orientation and multibyte support on, devkitPro's malloc lock
  hooks (CUSTOM_MALLOC_LOCK), no newlib-supplied system calls (tools/redhills/runtime provides them)
- libstdc++ from GCC 16.1.0, configured against that newlib, its headers being the ones the RedHills build
  compiles the game and tools/redhills/build_libstdcxx.py against

The result is a prefix with powerpc-eabi/include, powerpc-eabi/lib (libc.a, libm.a, ...) and
include/c++/16.1.0. It needs a POSIX shell, make and devkitPPC's powerpc-eabi-gcc (the build runs in the
devkitPPC container that tools/build.py uses).

    python tools/redhills/build_runtime16.py [--devkitpro DIR] [--out DIR] [--sources DIR] [-j N]
"""
import argparse
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))
import sources  # noqa: E402

NEWLIB_CONFIGURE = ['--disable-newlib-supplied-syscalls', '--enable-newlib-mb', '--enable-newlib-wide-orient',
                    '--enable-newlib-register-fini', '--disable-multilib', '--target=powerpc-eabi']
NEWLIB_CFLAGS = '-O2 -ffunction-sections -fdata-sections -DCUSTOM_MALLOC_LOCK -fshort-wchar'
LIBSTDCXX_CONFIGURE = ['--host=powerpc-eabi', '--target=powerpc-eabi', '--with-newlib', '--disable-multilib',
                       '--disable-shared', '--enable-threads=posix', '--disable-libstdcxx-pch',
                       '--disable-libstdcxx-verbose', '--enable-libstdcxx-time=yes',
                       '--enable-libstdcxx-filesystem-ts', '--disable-tm-clone-registry', '--disable-__cxa_atexit',
                       '--enable-cxx-flags=-fshort-wchar -ffunction-sections -fdata-sections']
LIBSTDCXX_CFLAGS = '-O2 -ffunction-sections -fdata-sections -fshort-wchar'


def run(command, cwd, env, log_dir, label):
    started = time.monotonic()
    log = log_dir / (label + '.log')
    with log.open('w', encoding='utf-8') as stream:
        result = subprocess.run(command, cwd=cwd, env=env, stdout=stream, stderr=subprocess.STDOUT)
    print('%-20s %s (%.0f s)' % (label, 'ok' if result.returncode == 0 else 'FAILED, see %s' % log,
                                 time.monotonic() - started), flush=True)
    if result.returncode:
        raise SystemExit(result.returncode)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    parser.add_argument('--devkitpro', type=Path, default=os.environ.get('DEVKITPRO', '/opt/devkitpro'),
                        help='the devkitPro root with devkitPPC (default $DEVKITPRO or /opt/devkitpro)')
    parser.add_argument('--out', type=Path, default=ROOT / 'build/runtime16', help='the install prefix')
    parser.add_argument('--sources', type=Path, default=sources.DEFAULT_CACHE,
                        help='the download and extraction cache of tools/redhills/sources.py')
    parser.add_argument('--work', type=Path, help='the build directory (default: <out>.work)')
    parser.add_argument('-j', '--jobs', type=int, default=os.cpu_count() or 4)
    args = parser.parse_args()

    compiler_bin = args.devkitpro / 'devkitPPC/bin'
    if not (compiler_bin / 'powerpc-eabi-gcc').is_file():
        sys.exit('powerpc-eabi-gcc not found in %s: install devkitPPC or pass --devkitpro' % compiler_bin)
    for tool in ('make', 'sh'):
        if shutil.which(tool) is None:
            sys.exit('%s is not on PATH' % tool)
    newlib_src = sources.newlib_tree(args.sources).resolve()
    gcc_src = sources.gcc_tree(args.sources).resolve()

    prefix = args.out.resolve()
    work = (args.work or args.out.with_name(args.out.name + '.work')).resolve()
    for path in (prefix, work):
        if path.exists():
            shutil.rmtree(path)
    (work / 'newlib').mkdir(parents=True)
    (work / 'libstdc++').mkdir()
    logs = work / 'logs'
    logs.mkdir()

    env = dict(os.environ)
    env['PATH'] = str(compiler_bin) + os.pathsep + env['PATH']
    make = ['make', '-j%d' % args.jobs, 'MAKEINFO=true']
    install = ['make', 'install', '-j1', 'MAKEINFO=true']

    newlib_env = dict(env, CFLAGS_FOR_TARGET=NEWLIB_CFLAGS)
    run([str(newlib_src / 'configure')] + NEWLIB_CONFIGURE + ['--prefix=' + str(prefix)], work / 'newlib',
        newlib_env, logs, 'configure-newlib')
    run(make, work / 'newlib', newlib_env, logs, 'build-newlib')
    run(install, work / 'newlib', newlib_env, logs, 'install-newlib')

    sysroot = prefix / 'powerpc-eabi'
    build_triple = subprocess.run(['sh', str(gcc_src / 'config.guess')], capture_output=True, text=True,
                                  check=True).stdout.strip()
    cxx_env = dict(env, CC='powerpc-eabi-gcc --sysroot=%s -fshort-wchar' % sysroot,
                   CXX='powerpc-eabi-g++ --sysroot=%s -fshort-wchar' % sysroot,
                   CFLAGS=LIBSTDCXX_CFLAGS, CXXFLAGS=LIBSTDCXX_CFLAGS, AR='powerpc-eabi-ar',
                   RANLIB='powerpc-eabi-ranlib')
    run([str(gcc_src / 'libstdc++-v3/configure'), '--build=' + build_triple, '--prefix=' + str(prefix)]
        + LIBSTDCXX_CONFIGURE, work / 'libstdc++', cxx_env, logs, 'configure-libstdc++')
    run(make, work / 'libstdc++', cxx_env, logs, 'build-libstdc++')
    run(install, work / 'libstdc++', cxx_env, logs, 'install-libstdc++')

    shutil.rmtree(work)
    print(prefix)
    return 0


if __name__ == '__main__':
    sys.exit(main())
