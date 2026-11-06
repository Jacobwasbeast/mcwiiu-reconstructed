# Minecraft: Wii U Edition - source reconstruction

C++ source for Minecraft: Wii U Edition (4J Studios' Legacy Console Edition
codebase), reconstructed as a decompilation. The source is written by
reading the original executable's machine code, and each function is A/B
tested against the original function and refined until it matches. Every
file is written source, not tool output: the repository contains no
binaries, assets, symbol tables, disassembly or decompiler output. See
`docs/provenance.md`.

So far, the source has been written by AI agents, directed and reviewed by
people. Contributions are open to anyone; see `docs/provenance.md` for the
rules.

```
git clone --recurse-submodules https://github.com/Jacobwasbeast/mcwiiu-reconstructed.git
python tools/prepare.py --base <base game dump> --update <update v688 dump>
python tools/build.py
python tools/package.py --update <update v688 dump>
Cemu -g build/title/code/Minecraft.Client.rpx
```

You need your own dumps of the base game and update 688. The build runs in
Docker and compiles with RedHills, an open-source clang fork that uses the
same C++ class layout as the compiler the game was built with. See
`docs/building.md`.

## Is this a static recompilation?

No. A static recompiler mechanically translates an executable's machine
code into code for another system. Its output typically emulates the
original CPU (guest registers, an emulated memory array, functions named by
address) and is regenerated from the executable for every build.

This project is a decompilation. The source is ordinary C++ with the
original classes, names and types, written to the rules in
`docs/conventions.md`. The build compiles it and links the executable
without any code from the original; the only things taken from your dump at
build time are shaders, game content and a few pieces of data such as keys
and lookup tables (see `docs/building.md`). Because it is ordinary source
with a platform seam (`docs/architecture.md`), it can be modded and ported
like any other C++ codebase.

## Status

The game builds and runs in a stock Cemu 2.6. It boots to the menus,
creates and loads worlds, and its server has run at the full 20 ticks per
second in testing. Reconstruction is ongoing, and not every function
matches the original yet. `docs/measuring.md` gives a rough progress
figure you can check against your own copy of the game.

- [docs/overview.md](docs/overview.md) - what is here, how it is written, and status
- [docs/provenance.md](docs/provenance.md) - where the source comes from, licensing and contribution rules
- [docs/building.md](docs/building.md) - preparing game files, building the RPX and running it in Cemu
- [docs/measuring.md](docs/measuring.md) - comparing your build with the original
- [docs/architecture.md](docs/architecture.md) - directory layout and the platform seam
- [docs/conventions.md](docs/conventions.md) - language level, naming and file rules

## Disclaimers

- This is an unofficial fan project, not affiliated with, endorsed by or
  supported by Mojang, Microsoft, 4J Studios or Nintendo. Minecraft is a
  trademark of Mojang Synergies AB; Wii U is a trademark of Nintendo.
- The repository contains no game content. You need your own copy of the
  game, dumped from your own console.
- The reconstruction does not match the original everywhere yet and may have
  bugs the original does not.
- It has been tested in Cemu only. Running it on a Wii U console is untested
  and at your own risk.
- Back up your saves. It uses the game's own save code, but a bug can still
  corrupt a world.
- Nintendo ended the Wii U's online services in April 2024. The online
  features of this build have not been tested.
- The software is provided as is, without warranty of any kind; see
  `LICENSE`.

## License

The GNU General Public License, version 3 or (at your option) any later
version, covers this project's own tooling: the build, packaging and
measuring tools under `tools/`. See `LICENSE`.

The reconstructed source under `source/` is derived from the original
executable, so it is not covered by that license, and nothing in this
repository grants any rights to Minecraft: Wii U Edition, its code, its
assets or its trademarks. See `docs/provenance.md`.

Files from other projects keep their own licenses:

- `tools/build/third_party/rapidxml/` - Boost Software License 1.0.
- `tools/build/overrides/` - altered libpng and curl files, under the libpng
  and curl licenses.
- The libraries `tools/build/deps.py` downloads - listed in
  `docs/building.md`.
- `source/nn/ffl` - a submodule holding aboood40091's FFL decompilation,
  which its author publishes without a license, plus this project's changes
  to it. It is not covered by this repository's license.
