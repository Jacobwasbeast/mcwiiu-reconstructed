# Source conventions

These are the rules every file under `source/` follows. They exist so the
tree reads as one codebase rather than as thousands of independent efforts.

## Language level

- C++03 as compiled by the Green Hills toolchain the original used. No
  `auto`, lambdas, range-for, `nullptr`, `override`, `constexpr`, rvalue
  references or `std::` types newer than C++03.
- boost 1.x for smart pointers, unordered containers and hashing
  (`boost::shared_ptr`, `boost::unordered_map`, `boost::hash`). The standard
  library covers everything else.
- No compiler extensions and no `#pragma pack`.

## One family, two files

- A class family is one header and one implementation file, both named
  after the outermost class: `Block.hpp` and `Block.cpp`.
- Nested classes, local helper classes and anonymous structs live inside
  their outer class's files.
- A template and all of its instantiations are one family.
- Header-only families (interfaces and enum owners with no symbols of their
  own) have a header and no implementation file.

## Naming

- Classes keep the names the build exposes: Mojang-style names for the game
  core (`LivingEntity`, `BlockPos`), 4J's `C`-prefixed names for the console
  layer (`CConsoleMinecraftApp`, `CStorage`) and `FJ_` / `fui` for the UI.
- Game-core members use the Mojang `mName` style; the 4J console and UI
  layers use `m_name`, with `m_p` for pointers; statics use `s` or `s_`
  to match their layer.
- Parameter, local and field names that the build does not record are
  chosen to read naturally in context. They are provisional and may be
  renamed as understanding improves.

## Content

- Files contain code only: no comments, no headers describing where the
  code came from, no annotations, no TODO or placeholder markers.
- No stubs. A method exists with a real body or it does not exist yet.
  A declaration with no definition is used only where the original build
  itself has none.
- No machine-generated idioms: no raw memory loads and stores standing in
  for member access, no hand-built vtables, no decompiler identifiers.

## Layout and includes

- Includes are `source/`-relative and sorted with the family's own header
  first, then project headers, then library and system headers.
- Forward declarations are preferred in headers; implementation files
  include what they use.
- Four-space indentation, braces on their own line for functions and
  classes, LF line endings, UTF-8 without BOM.
