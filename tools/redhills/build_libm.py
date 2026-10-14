"""Build newlib's libm with RedHills, for the Green Hills ABI build of the game.

The two-byte-wchar_t runtime (tools/redhills/build_runtime16.py) carries a libm compiled by GCC. GCC code
keeps condition register values across calls (mfcr around a call, for one), which Cafe emulators such as
Cemu 2.6 mishandle, and pow() does exactly that; the Green Hills math library never does. This rebuilds
every member of that libm from the same newlib sources and configuration with RedHills, whose Cafe target
keeps condition values local:

- sources: newlib 4.6.0.20260123 with devkitPro's patch (tools/redhills/sources.py), the release and patch
  the runtime is built from
- the member list: the runtime's libm.a, each libm_a-<name>.o compiled from <name>.c, looked up in the
  configured powerpc machine directory first, then libm/common, math, complex and fenv
- flags: the powerpc-eabi newlib_cflags of configure.host plus build_runtime16.py's CFLAGS_FOR_TARGET, in
  GNU C (gnu17: GCC 16's gnu23 default less its removal of K&R definitions, which w_drem.c uses;
  RedHills otherwise defaults to strict ANSI C, which hides finite and friends), and the include
  order of newlib's Makefile.am (libm/common, the configured target headers, then newlib's
  include directory after them)

    python tools/redhills/build_libm.py --redhills CLANG [--runtime16 DIR] [--src NEWLIB_TREE] [--out DIR]
                                        [-j N]
"""
import argparse
import concurrent.futures
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))
import sources  # noqa: E402
import toolchain  # noqa: E402

SEARCH = ['libm/machine/powerpc', 'libm/common', 'libm/math', 'libm/complex', 'libm/fenv']
NEWLIB_CFLAGS = ['-DHAVE_ASSERT_FUNC', '-DABORT_PROVIDED', '-DCUSTOM_MALLOC_LOCK', '-D__DYNAMIC_REENT__',
                 '-DGETREENT_PROVIDED', '-DREENTRANT_SYSCALLS_PROVIDED', '-D__DEFAULT_UTF8__']
FLAGS = ['-target', 'powerpc-eabi', '-mcpu=750', '-std=gnu17', '-O2', '-ffunction-sections', '-fdata-sections', '-fshort-wchar',
         '-w', '-nostdlibinc'] + NEWLIB_CFLAGS


def members(llvm_ar, runtime16):
    listing = subprocess.run([str(llvm_ar), 't', str(runtime16 / 'powerpc-eabi/lib/libm.a')], capture_output=True,
                             text=True, check=True).stdout
    return [line.strip() for line in listing.splitlines() if line.strip()]


def source_of(newlib, member):
    stem = member[len('libm_a-'):-len('.o')] if member.startswith('libm_a-') else member[:-len('.o')]
    for directory in SEARCH:
        path = newlib / directory / (stem + '.c')
        if path.is_file():
            return path
    return None


def compile_member(clang, newlib, out, runtime16, member):
    source = source_of(newlib, member)
    if source is None:
        return member, 'no source for this member'
    obj = out / 'obj' / member
    command = [str(clang)] + FLAGS + ['-I' + str(newlib / 'libm/common'), '-I' + str(source.parent),
                                      '-isystem', str(runtime16 / 'powerpc-eabi/include'),
                                      '-idirafter', str(newlib / 'include'), '-c', str(source), '-o', str(obj)]
    result = subprocess.run(command, capture_output=True, text=True, errors='replace')
    if result.returncode:
        lines = [line for line in result.stderr.splitlines() if 'error' in line]
        return member, (lines or [result.stderr.strip()])[0]
    return member, None


def main():
    parser = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    parser.add_argument('--redhills', type=Path, default=os.environ.get('REDHILLS'),
                        help='the RedHills clang executable (default $REDHILLS)')
    parser.add_argument('--runtime16', type=Path, default=toolchain.default_runtime16(),
                        help='the two-byte wchar_t runtime (tools/redhills/build_runtime16.py)')
    parser.add_argument('--src', type=Path, help='a prepared newlib tree (default: tools/redhills/sources.py)')
    parser.add_argument('--sources', type=Path, default=sources.DEFAULT_CACHE,
                        help='the download and extraction cache of tools/redhills/sources.py')
    parser.add_argument('--out', type=Path, default=ROOT / 'build/redhills/libm')
    parser.add_argument('-j', '--jobs', type=int, default=os.cpu_count() or 4)
    args = parser.parse_args()
    if not args.redhills or not args.redhills.is_file():
        sys.exit('RedHills clang not found: pass --redhills or set REDHILLS')
    if not (args.runtime16 / 'powerpc-eabi/lib/libm.a').is_file():
        sys.exit('two-byte wchar_t runtime not found at %s (tools/redhills/build_runtime16.py)' % args.runtime16)
    src = args.src or sources.newlib_tree(args.sources)
    newlib = src / 'newlib'
    if not (newlib / 'libm/math/w_pow.c').is_file():
        sys.exit('newlib sources not found at %s' % src)
    llvm_ar = toolchain.llvm_tool('llvm-ar', args.redhills)
    (args.out / 'obj').mkdir(parents=True, exist_ok=True)
    names = members(llvm_ar, args.runtime16)
    failures = []
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        for member, error in pool.map(lambda name: compile_member(args.redhills, newlib, args.out, args.runtime16,
                                                                  name), names):
            if error:
                failures.append((member, error))
    for member, error in failures:
        print('FAIL %-32s %s' % (member, error))
    archive = args.out / 'libm.a'
    if archive.exists():
        archive.unlink()
    response = args.out / 'libm.rsp'
    response.write_text(''.join('"%s"\n' % (args.out / 'obj' / name).as_posix() for name in names
                                if (args.out / 'obj' / name).is_file()), encoding='utf-8')
    subprocess.run([str(llvm_ar), 'rcs', str(archive), '@%s' % response], check=True)
    print('%d of %d members compiled -> %s' % (len(names) - len(failures), len(names), archive))
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
