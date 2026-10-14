#!/usr/bin/env python3
"""Build Minecraft.Client.rpx for the Wii U with RedHills.

    python tools/build.py              build in Docker (any host with Docker)
    python tools/build.py --local      build with tools installed on this machine

Run tools/prepare.py once first; the build needs the shaders it generates.

The game is compiled with RedHills (https://github.com/Jacobwasbeast/RedHills), a clang that follows the
class ABI the original game was built with, and linked the way the retail image is (see
tools/rpx/build_redhills.py). The open-source libraries it uses are downloaded from their official
releases, checked against pinned hashes and cached in build/deps.

In Docker, the first build creates the toolchain image (tools/build/Dockerfile): it compiles RedHills,
which takes from 30 minutes to a few hours depending on the machine. Later builds reuse the image and
the build cache in the Docker volume, so they recompile only what changed.

The result is build/Minecraft.Client.rpx, with its link map beside it.

Options:
  --jobs N       parallel compile jobs (default: all cores)
  --clean        discard the cached build (the toolchain image is kept)
  --keep-going   compile everything that can be compiled and report every failure
"""
import argparse
import hashlib
import os
import shutil
import subprocess
import sys
import time

TOOLS = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(TOOLS)
sys.path.insert(0, os.path.join(TOOLS, "build"))
import deps  # noqa: E402

DOCKERFILE = os.path.join(TOOLS, "build", "Dockerfile")
IMAGE_NAME = "minecraft-wiiu-redhills"
VOLUME = "minecraft-wiiu-build"
BUILD_DIR = os.path.join(ROOT, "build")
DEPS_DIR = os.path.join(BUILD_DIR, "deps")
SHADER_DIR = os.path.join(ROOT, "source", "platform", "wiiu", "shaders")
GENERATED = ("RendererShaders.inc", "MiiIconDefaultShader.inc", "MiiIconExtShader.inc", "MonitoringCryptoKeys.inc",
             "PeadSjisCodeTable.inc", "PeadSjisIndexTable.inc", "PeadFloatDigitTable.inc")
STAGED = ("source", "tools")
EXCLUDED = (".git", "__pycache__", ".download-cache")
# The two-byte wchar_t runtime is rebuilt when the scripts that define it change.
RUNTIME16_INPUTS = ("tools/redhills/build_runtime16.py", "tools/redhills/sources.py")
LIBGCC = "devkitPPC/lib/gcc/powerpc-eabi/16.1.0/libgcc.a"


class BuildError(Exception):
    """The build cannot start or did not succeed."""


def run(command, **kwargs):
    print("+ " + " ".join(str(part) for part in command), flush=True)
    result = subprocess.run([str(part) for part in command], **kwargs)
    if result.returncode != 0:
        raise BuildError("command failed with exit code %d: %s" % (result.returncode, command[0]))
    return result


def check_prepared():
    missing = [name for name in GENERATED if not os.path.isfile(os.path.join(SHADER_DIR, name))]
    if missing:
        raise BuildError("the files tools/prepare.py generates are missing (%s). Run tools/prepare.py with your base game "
                         "and update dumps first." % ", ".join(missing))
    if not os.path.isfile(os.path.join(ROOT, "source", "nn", "ffl", "FFLiManager.cpp")):
        raise BuildError("source/nn/ffl is empty. Fetch the FFL submodule: git submodule update --init")


def content_key(paths):
    digest = hashlib.sha256()
    for path in paths:
        with open(os.path.join(ROOT, path), "rb") as handle:
            digest.update(path.encode("utf-8") + b"\0" + handle.read().replace(b"\r\n", b"\n"))
    return digest.hexdigest()[:16]


def build_arguments(python, root, deps_dir, third_party, runtime16, sources, out, libgcc, jobs, keep_going):
    """The commands that lay out the third-party sources, build the two-byte wchar_t runtime when its
    stamp is stale, and build the RPX, as one POSIX shell script."""
    key = content_key(RUNTIME16_INPUTS)
    stamp = runtime16 + "/.stamp"
    rpx = [python, root + "/tools/rpx/build_redhills.py", "--libgcc", libgcc, "--wut", deps_dir + "/wut",
           "--runtime16", runtime16, "--third-party", third_party, "--sources", sources, "--out", out]
    if jobs:
        rpx += ["--jobs", str(jobs)]
    if keep_going:
        rpx.append("--keep-going")
    runtime = [python, root + "/tools/redhills/build_runtime16.py", "--out", runtime16, "--sources", sources]
    if jobs:
        runtime += ["-j", str(jobs)]
    return " && ".join([
        " ".join([python, root + "/tools/build/layout.py", deps_dir, third_party]),
        '{ [ "$(cat %s 2>/dev/null)" = "%s" ] || { %s && echo %s > %s; }; }'
        % (stamp, key, " ".join(runtime), key, stamp),
        " ".join(rpx)])


def host_tar():
    """The system tar. Windows 10 and later ship bsdtar in System32, which takes
    Windows paths; a tar found earlier on PATH (Git's, for one) may not."""
    if os.name == "nt":
        system = os.path.join(os.environ.get("SystemRoot", r"C:\Windows"), "System32", "tar.exe")
        if os.path.isfile(system):
            return system
    found = shutil.which("tar")
    if found is None:
        raise BuildError("tar is not on PATH")
    return found


def image_tag():
    with open(DOCKERFILE, "rb") as handle:
        return "%s:%s" % (IMAGE_NAME, hashlib.sha256(handle.read().replace(b"\r\n", b"\n")).hexdigest()[:16])


def ensure_image():
    tag = image_tag()
    present = subprocess.run(["docker", "image", "inspect", tag], stdout=subprocess.DEVNULL,
                             stderr=subprocess.DEVNULL).returncode == 0
    if not present:
        print("building the toolchain image %s (compiles RedHills; this takes a while once)" % tag, flush=True)
        with open(DOCKERFILE, "rb") as handle:
            result = subprocess.run(["docker", "build", "-t", tag, "-"], stdin=handle)
        if result.returncode != 0:
            raise BuildError("building the toolchain image failed with exit code %d" % result.returncode)
    return tag


def docker(image, *command, mounts=(), capture=False):
    args = ["docker", "run", "--rm", "-v", VOLUME + ":/work"]
    for host, target in mounts:
        args += ["-v", "%s:%s" % (host, target)]
    args += [image] + list(command)
    if capture:
        return subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print("+ docker run %s %s" % (image, " ".join(command)), flush=True)
    result = subprocess.run(args)
    if result.returncode != 0:
        raise BuildError("the build in Docker failed with exit code %d" % result.returncode)
    return result


def stage(image, directory, names, destination):
    """Replace /work/<destination> in the volume with `names` under `directory`.

    The host's tar streams the files into the container, which is far faster
    than sharing the folder with Docker, and keeps their modification times, so
    the build recompiles only what changed. The copy lands beside the destination
    and is renamed over it only once it is complete, so an interrupted run never
    leaves a partial tree that a later run would trust."""
    target = "/work/" + destination
    script = ("rm -rf {0}.partial && mkdir -p {0}.partial && tar -x -C {0}.partial && "
              "rm -rf {0} && mv {0}.partial {0}").format(target)
    pack = [host_tar(), "-C", directory, "-cf", "-"]
    for name in EXCLUDED:
        pack += ["--exclude", name]
    packer = subprocess.Popen(pack + list(names), stdout=subprocess.PIPE)
    try:
        unpacked = subprocess.run(["docker", "run", "--rm", "-i", "-v", VOLUME + ":/work", image, "sh", "-c", script],
                                  stdin=packer.stdout)
    finally:
        packer.stdout.close()
        packed = packer.wait()
    if packed != 0:
        raise BuildError("tar failed with exit code %d while reading %s" % (packed, directory))
    if unpacked.returncode != 0:
        raise BuildError("copying %s into the Docker volume failed with exit code %d"
                         % (destination, unpacked.returncode))


def copy_outputs(out):
    os.makedirs(BUILD_DIR, exist_ok=True)
    shutil.copyfile(os.path.join(out, "Minecraft.Client.rpx"), os.path.join(BUILD_DIR, "Minecraft.Client.rpx"))
    shutil.copyfile(os.path.join(out, "minecraft.map"), os.path.join(BUILD_DIR, "Minecraft.Client.map"))


def build_in_docker(jobs, clean, keep_going):
    if shutil.which("docker") is None:
        raise BuildError("docker is not installed or not on PATH; install Docker or use --local")
    if subprocess.run(["docker", "info"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode != 0:
        raise BuildError("the Docker daemon is not running")
    image = ensure_image()
    if clean:
        subprocess.run(["docker", "volume", "rm", "-f", VOLUME], stdout=subprocess.DEVNULL)

    with open(os.path.join(DEPS_DIR, deps.STAMP), "rb") as handle:
        wanted = handle.read()
    staged = docker(image, "cat", "/work/deps/" + deps.STAMP, capture=True)
    if staged.returncode != 0 or staged.stdout != wanted:
        print("copying the libraries into the Docker volume", flush=True)
        stage(image, DEPS_DIR, ["."], "deps")

    print("copying source/ and tools/ into the Docker volume", flush=True)
    stage(image, ROOT, STAGED, "repo")

    script = build_arguments("python3", "/work/repo", "/work/deps", "/work/third_party", "/work/runtime16",
                             "/work/sources", "/work/out", "/opt/devkitpro/" + LIBGCC, jobs, keep_going)
    docker(image, "sh", "-c", script)
    os.makedirs(BUILD_DIR, exist_ok=True)
    docker(image, "sh", "-c", "mkdir -p /out/redhills && cp /work/out/Minecraft.Client.rpx /work/out/minecraft.map "
                              "/out/redhills/", mounts=[(BUILD_DIR, "/out")])
    copy_outputs(os.path.join(BUILD_DIR, "redhills"))


def build_locally(jobs, clean, keep_going):
    devkitpro = os.environ.get("DEVKITPRO")
    if not devkitpro:
        raise BuildError("DEVKITPRO is not set; install devkitPPC, or drop --local to build in Docker")
    if not os.environ.get("REDHILLS"):
        raise BuildError("REDHILLS is not set; build RedHills (docs/building.md) and point REDHILLS at its clang")
    out = os.path.join(BUILD_DIR, "redhills")
    if clean and os.path.isdir(out):
        shutil.rmtree(out)
    script = build_arguments(sys.executable, ROOT, DEPS_DIR, os.path.join(BUILD_DIR, "third_party"),
                             os.path.join(BUILD_DIR, "runtime16"), os.path.join(BUILD_DIR, "redhills-sources"),
                             out, os.path.join(devkitpro, LIBGCC), jobs, keep_going)
    if shutil.which("sh") is None:
        raise BuildError("--local needs a POSIX shell and make (on Windows, MSYS2 or Git Bash with make)")
    run(["sh", "-c", script.replace("\\", "/")])
    copy_outputs(out)


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--local", action="store_true", help="build with RedHills and devkitPPC on this machine")
    parser.add_argument("--jobs", type=int, default=0, help="parallel compile jobs (default: all cores)")
    parser.add_argument("--clean", action="store_true", help="discard the cached build first")
    parser.add_argument("--keep-going", action="store_true", help="report every failure instead of stopping")
    args = parser.parse_args(argv)
    started = time.time()
    try:
        check_prepared()
        deps.fetch(DEPS_DIR)
        if args.local:
            build_locally(args.jobs, args.clean, args.keep_going)
        else:
            build_in_docker(args.jobs, args.clean, args.keep_going)
    except (BuildError, deps.DependencyError, OSError) as error:
        print("error: %s" % error, file=sys.stderr)
        return 1
    rpx = os.path.join(BUILD_DIR, "Minecraft.Client.rpx")
    print("built %s (%.1f MB) in %.0f minutes" % (os.path.relpath(rpx, ROOT), os.path.getsize(rpx) / 1e6,
                                                (time.time() - started) / 60))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
