# Overview

This repository holds a reconstructed C++ source tree for Minecraft: Wii U
Edition, the 4J Studios Legacy Console Edition codebase as it shipped on
Wii U. It is written from the structural evidence the unstripped Wii U
build leaves behind (class names, method signatures, class layouts, string
tables and control flow), cross-checked against the behaviour of the Xbox 360
TU75 build. Every file was authored as source; nothing is emitted by a
decompiler or copied from a game binary. See `provenance.md`.

## Who is writing it

All of the code in this repository is AI-constructed. Two dispatcher AIs
coordinate the work: Sol dispatches Luna worker agents and Fable dispatches
Opus 5 worker agents. Each worker is handed one class family at a time with
the evidence for that family, writes the header and implementation, and
returns them to its dispatcher, which runs the mechanical checks, records
the result and commits the accepted files. No human writes source here;
people direct the campaign, set the rules in `conventions.md`, and review.

## What is here

- `source/game/` - the platform-independent game: world, blocks, entities,
  items, networking, UI, rendering and audio.
- `source/platform/wiiu/` - the Nintendo backend of 4J's platform layer
  (threads, storage, profiles, input, online sessions).
- `source/vendor/` - reconstructed internals of third-party libraries the
  title links against.
- `source/nn/` - reconstructed internals of the NEX and Pia online libraries.
- `docs/` - these notes.

## What is not here

No game assets, textures, models, sounds, fonts, save data, executables,
symbol tables, disassembly, or decompiler output are committed to this
repository, and none should ever be.

## Status

The tree is reconstructed class by class. A class is present when both its
header and implementation exist under `source/`.
