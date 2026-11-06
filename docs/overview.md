# Overview

This repository holds reconstructed C++ source for Minecraft: Wii U
Edition, 4J Studios' Legacy Console Edition codebase as it shipped on Wii U.

It is a decompilation. The source is written by reading the original Wii U
executable's machine code, along with the symbols, type information and
string tables the unstripped build carries. Each function is then A/B
tested against the original function and refined until it matches. The Xbox
360 TU75 build is used as a cross-check for behaviour. Every file is written
source, not tool output. See `provenance.md`.

## How it is written

So far, all of the source has been written by AI agents. People direct the
work, set the rules in `conventions.md` and review the results.
Contributions are open to anyone, with or without AI tools; see
`provenance.md` for the rules.

Claude coordinates the work, giving Claude Opus 5.5 worker agents one task
at a time along with the evidence for it: a class family to rebuild, or a
set of functions whose compiled code differs from the original. Each change
is compiled, A/B tested function by function against the original
executable and independently audited before it is accepted. Earlier phases
worked the same way with other agents: the dispatchers Sol and Fable handed
class families to Luna and Opus 5 workers.

## What is here

- `source/game/` - the platform-independent game: world, blocks, entities,
  items, networking, UI, rendering and audio.
- `source/platform/wiiu/` - the Wii U implementation of 4J's platform layer:
  threads, storage, profiles, input, online sessions and the Mii icon
  module.
- `source/vendor/` - reconstructed internals of third-party libraries the
  game links: Miles audio, Bink audio, the voice-chat codec, dlmalloc and
  stb_truetype.
- `source/nn/` - reconstructed internals of Nintendo's NEX and Pia online
  libraries and the small SDK libraries the game links (crypto, software
  keyboard, error viewer), plus the Mii face library in the `source/nn/ffl`
  submodule.
- `source/cafe/` - reconstructed Cafe SDK pieces: GX2 and GFD declarations,
  the matrix library, the C runtime's system layer and the C++ runtime
  support.
- `docs/` - these notes.
- `tools/` - the build and measuring tools:
  - `prepare.py` sets up the game files from your dumps.
  - `build.py` builds the RPX with RedHills.
  - `package.py` lays it out as a title folder for Cemu.
  - `tools/rpx/` and `tools/redhills/` link the executable the way the
    original was linked.
  - `tools/build/` holds the pinned library list, the toolchain image and
    the few altered library files.
  - `tools/measure/` compares a build with the original executable.

## What is not here

No game assets, textures, models, sounds, fonts, save data, executables,
compiled shaders, symbol tables, disassembly or decompiler output. None of
these should ever be committed.

## Status

`tools/build.py` compiles the whole tree and links `Minecraft.Client.rpx`
without any code from the original executable. Shaders, game content and a
few pieces of data (keys and lookup tables) are extracted from your own
dump at build time (see `building.md`).

The result runs in a stock Cemu 2.6. It boots to the menus, creates and
loads worlds, and its server has run at the full 20 ticks per second in
testing. It has not been tried on a Wii U console. Nintendo ended the Wii
U's online services in April 2024, and the online features have not been
tested.

Reconstruction is ongoing, and not every function matches the original yet.
If you find a behaviour difference, report it as a bug. For a rough
progress figure, `tools/measure/parity.py` compares your build with your
copy of the original; see `measuring.md`.
