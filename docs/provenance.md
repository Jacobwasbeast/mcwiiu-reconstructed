# Provenance

## Where the source comes from

This repository is a decompilation of the Wii U build of the game. The
source is derived from the original executable: it is written by reading the
executable's machine code, then A/B tested against the original function by
function and refined until each function matches.

The Wii U build ships unstripped, which gives the source its shape:

- the symbol table names every function and class;
- the run-time type information records the class hierarchy;
- constructors and string tables record class layouts and the text the game
  uses.

The function bodies come from the machine code itself, read as disassembly
and as intermediate listings generated from it.

Each class family was reconstructed as a unit by a worker agent (see
`overview.md`). The agent read the evidence for the class, including its
machine code, worked out what the original source must have looked like,
and wrote it in the style set out in `conventions.md`. Each function was
then A/B tested against the original and refined until the two matched.
Where the Wii U evidence alone left a question open, the Xbox 360 TU75
build of the same codebase was used as the reference for behaviour.

## Written source, not tool output

No file under `source/` is the output of a decompiler or of any other tool
run on the binary. Disassembly and listings were read as evidence; the
source was written from what they showed. The working repository that held
that evidence and the reconstruction workflow is kept separate. Only the
resulting source, and the tooling needed to build it from your own copy of
the game, is published here.

## What this repository excludes

- The game binaries, in any form.
- Game assets: textures, models, sounds, fonts, UI movies, language files,
  world templates and save data.
- Symbol tables, disassembly, decompiler output, intermediate "lowered" or
  "semantic" source, and any other file generated from the binary.
- Compiler and linker metadata recovered from the build.
- The tooling that consumed those inputs.

## Licensing

The game, its code and its content belong to their owners. Because the
reconstructed source is derived from the original executable, the GPL in
`LICENSE` does not cover it. The GPL covers this project's own tooling: the
build, packaging and measuring tools under `tools/`. Nothing in this
repository grants any rights to Minecraft: Wii U Edition. Build it only from
your own copy of the game.

Code from other projects keeps its own license, including the altered
libpng and curl files in `tools/build/overrides/`. The README lists them.

## Third-party libraries

`source/vendor/` and the library parts of `source/nn/` reconstruct the
internals of libraries the game links, because those libraries were built
into the executable with their names intact and the game calls into them.
They are kept apart from the game code so the boundary is explicit. If a
maintained upstream becomes available for any of them, that upstream should
replace the reconstructed copy.

The Mii face library is the one case where an upstream already exists:
aboood40091's FFL decompilation. Its code is not copied into this
repository. It is included as the `source/nn/ffl` submodule, a fork of that
project with this reconstruction's changes as separate commits, so
upstream's authorship stays with upstream.

## Contributions

Anyone can contribute, with or without AI tools. The rules:

- Write source from your understanding of the original. You may gain that
  understanding by reading the original's machine code, but never paste
  decompiler output.
- If you use AI tools, do not add co-author trailers (such as
  `Co-Authored-By`) for them. You are the author of what you submit and take
  full responsibility for it.
- Follow `conventions.md`.
- Never include binaries, assets or anything from the excluded list above.
  A pull request that does will be closed.
