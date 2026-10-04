# Provenance

## Where this source comes from

The Wii U build of the title ships unstripped. Its symbol table names every
function and every class, its run-time type information records the class
hierarchy, and its constructors and string tables record class layouts and
the text the game uses. That is structural evidence: it says what the source
was shaped like, not what it said.

This repository is the source written back from that shape, and all of it
is written by AI agents (see `overview.md`). Each class was reconstructed
as a unit by a worker agent reading the evidence for that class, deciding
what the original source must have looked like, and writing it in the style
described in `conventions.md`. The Xbox 360 TU75 build of the same
codebase served as the behavioural reference when the Wii U evidence alone
left a question open.

## What this repository deliberately excludes

- The game binaries, in any form.
- Game assets: textures, models, sounds, fonts, UI movies, language files,
  world templates and save data.
- Symbol tables, disassembly, decompiler output, intermediate "lowered" or
  "semantic" source, and any other file generated from the binary.
- Compiler and linker metadata recovered from the build.
- The tooling that consumed those inputs.

The reconstruction workflow that produced this tree lived in a separate
working repository alongside that evidence. Only the resulting source, and
the small amount of tooling needed to check it, was carried across. Nothing
under `source/` is a transformation of a binary; every file is authored
text.

## Third-party code

`source/vendor/` reconstructs the internals of libraries the title links
against, because those libraries were built into the executable with their
names intact and game code calls into them. They are kept separate from the
game so that the boundary is explicit. If a maintained upstream for any of
them becomes available, the reconstructed copy should be replaced by that
upstream rather than maintained here.

The Mii face library is the one case where such an upstream exists:
aboood40091's FFL decompilation. Its code is never copied into this
repository. It is referenced as the `source/nn/ffl` submodule, a fork of
that project that carries this reconstruction's changes as separate
commits, so upstream's authorship stays with upstream.

## Contributions

Contributions must follow the same rule: source written from understanding,
never pasted from a decompiler, and never accompanied by binaries or assets.
A pull request that adds anything from the excluded list will be closed.
