# The toolchain tools/build.py builds the RPX with: devkitPPC (for its GCC, which builds the two-byte
# wchar_t C and C++ runtime, and its libgcc), RedHills (the Green Hills ABI clang the game is compiled
# with), and LLVM's linker and archive tools.
FROM devkitpro/devkitppc@sha256:44cb1a920e1ec3ec7c06767493c3b85f8d643d6137cc4661f0201895ac6e4967

ARG REDHILLS_URL=https://github.com/Jacobwasbeast/RedHills.git
ARG REDHILLS_COMMIT=37713dd04a8cc1d211749b0abdcc684a8aa11423
ARG LLVM_RELEASE=https://github.com/llvm/llvm-project/releases/download/llvmorg-22.1.8/LLVM-22.1.8-Linux-X64.tar.xz
ARG LLVM_SHA256=df0e1ecf16caf3489a272a5eea4eec9b0d82878f6477fa309504f918a0006384
ARG CAPSTONE_VERSION=5.0.7

RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates curl lld python3-pip xz-utils \
 && rm -rf /var/lib/apt/lists/* \
 && pip3 install --no-cache-dir --break-system-packages capstone==${CAPSTONE_VERSION}

# RedHills: clang only, configured as the RedHills Linux setup script does, with the assertions the
# compiler is tested with.
RUN git init -q /tmp/redhills \
 && git -C /tmp/redhills remote add origin ${REDHILLS_URL} \
 && git -C /tmp/redhills fetch -q --depth 1 origin ${REDHILLS_COMMIT} \
 && git -C /tmp/redhills checkout -q FETCH_HEAD \
 && cmake -S /tmp/redhills/llvm -B /tmp/redhills/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS=clang -DLLVM_TARGETS_TO_BUILD=PowerPC \
      -DLLVM_DEFAULT_TARGET_TRIPLE=powerpc-eabi -DLLVM_ENABLE_ASSERTIONS=ON -DLLVM_USE_LINKER=lld \
      -DLLVM_PARALLEL_LINK_JOBS=2 -DLLVM_OPTIMIZED_TABLEGEN=ON \
      -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_TESTS=OFF -DCLANG_INCLUDE_TESTS=OFF \
      -DCLANG_ENABLE_ARCMT=OFF -DCLANG_PLUGIN_SUPPORT=OFF -DLLVM_ENABLE_BINDINGS=OFF \
      -DCLANG_ENABLE_STATIC_ANALYZER=OFF -DCLANG_ENABLE_OBJC_REWRITER=OFF \
      -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_ZLIB=OFF -DLLVM_ENABLE_LIBXML2=OFF \
 && ninja -C /tmp/redhills/build clang \
 && mkdir -p /opt/redhills/bin /opt/redhills/lib \
 && cp -a /tmp/redhills/build/bin/clang /tmp/redhills/build/bin/clang-[0-9]* /opt/redhills/bin/ \
 && cp -a /tmp/redhills/build/lib/clang /opt/redhills/lib/ \
 && rm -rf /tmp/redhills

# ld.lld, llvm-ar and llvm-nm from the LLVM 22.1.8 release, the versions the build is tested with.
RUN curl -fsSL -o /tmp/llvm.tar.xz ${LLVM_RELEASE} \
 && echo "${LLVM_SHA256}  /tmp/llvm.tar.xz" | sha256sum -c - \
 && mkdir -p /opt/llvm \
 && tar -xJf /tmp/llvm.tar.xz -C /opt/llvm --strip-components=1 \
      LLVM-22.1.8-Linux-X64/bin/lld LLVM-22.1.8-Linux-X64/bin/llvm-ar LLVM-22.1.8-Linux-X64/bin/llvm-nm \
 && rm /tmp/llvm.tar.xz \
 && ln -s lld /opt/llvm/bin/ld.lld \
 && /opt/llvm/bin/ld.lld --version && /opt/llvm/bin/llvm-ar --version | head -n 2

ENV REDHILLS=/opt/redhills/bin/clang \
    LLVM_BIN=/opt/llvm/bin \
    DEVKITPRO=/opt/devkitpro
