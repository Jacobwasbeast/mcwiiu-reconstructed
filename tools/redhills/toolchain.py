"""Locate the LLVM tools the RedHills build drives besides the compiler.

RedHills itself builds only clang; the linker and the archive tools come from an LLVM release
(the build is tested with LLVM 22's ld.lld, llvm-ar and llvm-nm). A tool is looked up, in order:
next to the RedHills clang, in $LLVM_BIN, on PATH, and in the default Windows install directory.
"""
import os
import shutil
from pathlib import Path

WINDOWS_DEFAULT = Path('C:/Program Files/LLVM/bin')
ROOT = Path(__file__).resolve().parents[2]
# Where tools/redhills/build_runtime16.py installs the two-byte wchar_t runtime, then an older location.
RUNTIME16_LOCATIONS = (ROOT / 'build/runtime16', ROOT / 'work/providers/wchar16-runtime-v2')


def default_runtime16():
    for path in RUNTIME16_LOCATIONS:
        if (path / 'powerpc-eabi/lib/libc.a').is_file():
            return path
    return RUNTIME16_LOCATIONS[0]


def executable_name(name):
    return name + '.exe' if os.name == 'nt' else name


def llvm_tool(name, clang=None):
    """The path of an LLVM tool such as 'ld.lld' or 'llvm-ar'; raises SystemExit when none is found."""
    filename = executable_name(name)
    candidates = []
    if clang:
        candidates.append(Path(clang).resolve().parent / filename)
    if os.environ.get('LLVM_BIN'):
        candidates.append(Path(os.environ['LLVM_BIN']) / filename)
    found = shutil.which(name)
    if found:
        candidates.append(Path(found))
    if os.name == 'nt':
        candidates.append(WINDOWS_DEFAULT / filename)
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    raise SystemExit('%s not found: install LLVM 22 (ld.lld, llvm-ar, llvm-nm) and put it on PATH or set LLVM_BIN'
                     % name)
