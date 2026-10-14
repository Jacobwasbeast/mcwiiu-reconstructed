# Minecraft: Wii U Edition - source reconstruction

Reconstructed C++ source for Minecraft: Wii U Edition, the 4J Studios
Legacy Console Edition codebase. Every file is authored source; the
repository contains no binaries, assets, symbol tables or decompiler output.

All of the code is written by AI agents. Today Claude coordinates Claude
Opus 5.5 worker agents; earlier phases had the dispatchers Sol and Fable
hand class families to Luna and Opus 5 workers. See `docs/overview.md`.

```
git clone --recurse-submodules https://github.com/Jacobwasbeast/mcwiiu-reconstructed.git
python tools/prepare.py --base <base game dump> --update <update v688 dump>
python tools/build.py
python tools/package.py --update <update v688 dump>
Cemu -g build/title/code/Minecraft.Client.rpx
```

The game files come from your own dumps of the base game and update 688.
The build compiles the game with RedHills, an open-source clang that keeps
the class layout of the compiler the game was built with, and runs in
Docker. See `docs/building.md`.

- [docs/overview.md](docs/overview.md) - what is here, what is not, and status
- [docs/building.md](docs/building.md) - preparing game files, building the RPX and running it in Cemu
- [docs/architecture.md](docs/architecture.md) - directory layout and the platform seam
- [docs/conventions.md](docs/conventions.md) - language level, naming and file rules
- [docs/provenance.md](docs/provenance.md) - where the source comes from and contribution rules
