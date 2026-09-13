# Architecture

The title is one C++ codebase with a platform abstraction in the middle.
The game core above the seam is platform-pure. The platform backend below
it is the only code that talks to the Cafe OS, the GX2 graphics API, the
VPAD/WPAD controllers and Nintendo's online services. The reconstruction
keeps that shape, and the directory layout is the seam made visible.

## Include root

Every include is written relative to `source/`, for example
`#include "game/world/block/Block.hpp"`. There are no relative includes and
no umbrella headers. A translation unit includes exactly the headers it
uses.

## `source/game/` - the game core

| Directory | Contents |
| --- | --- |
| `game/core` | The engine spine: `Minecraft`, services shared by client and server, `GlStateManager`, achievements and statistics, streams, the console application shell (`CConsoleMinecraftApp`), options, resources, and the many small value classes the rest of the game leans on. |
| `game/util` | Maths and geometry (`AABB`, `Vec3`, `Mth`, `Random`), resource locations, UUIDs and other freestanding helpers. |
| `game/data` | The NBT tag family (`CompoundTag`, `ListTag`, typed tags) and data fixers. |
| `game/world` | `Level`, `LevelSource`, dimensions, explosions, lighting and the save format. |
| `game/world/block` | `Block`, `Blocks`, block states and properties, block entities, materials and `BlockPos`. |
| `game/world/chunk` | Chunk storage, chunk sources, compressed block storage and the region file format. |
| `game/world/biome` | Biomes, biome sources and biome-driven decoration. |
| `game/world/generation` | Chunk generators, structures, features, world templates and the level-type catalogue. |
| `game/entity` | `Entity`, `LivingEntity`, players and the per-kind subdirectories `mob`, `ai` and `projectile`. |
| `game/item` | `Item`, `Items`, `ItemInstance`, inventories, containers, recipes and enchantments. |
| `game/network` | `Packet` and every packet type, packet listeners, connections and the session protocol. |
| `game/server` | The integrated server: chunk cache, command dispatcher, scoreboard, boss events. |
| `game/command` | Console commands and their argument parsing. |
| `game/minigame` | Battle, Tumble and Glide: rule definitions, medals, arenas and minigame state. |
| `game/ui` | The 4J user interface layer (`FJ_*`, `fui*`): screens, controls, HUD, menus and the UI node graph. |
| `game/client/render` | Renderers, particles, models, texture packs and the block/entity render dispatchers. |
| `game/client/audio` | The sound engine front end, music and sound events. |

## `source/platform/wiiu/` - the Nintendo backend

4J's platform layer is a set of `C4J*` and `C*` interfaces implemented once
per console. This directory holds the Wii U implementations: `C4JThread`,
`C4JEvent`, `C4JStorage`, `CStorage`, `CProfile`, `CInput`,
`CPlatformNetworkManagerNintendo` and the allocator glue for the Mii face
library. These files call Cafe OS and Nintendo SDK functions directly; no
file under `game/` does.

## `source/vendor/` - third-party internals

Internals of libraries the title links against and that the build exposes
with full names: the Mii face library (`FFLi*`), the NEX and Pia online
transport (`transport`, `RendezVous`, `chat`, `crypto`), and small
SDK-facing structures. They are kept apart so that game code is never
mistaken for library code.

## `source/nn/` - NEX and Pia library internals

`nn/nex/` and `nn/pia/` (with `chat`, `common`, `inet`, `lan`, `session`
and `transport` beneath it) hold the reconstructed internals of Nintendo's NEX
and Pia online libraries, which the title links statically and calls from
the platform backend and the vendor code. They are the same category as
`source/vendor/`: reconstructed from the symbols and structure the build
exposes, kept apart from the game so the boundary stays explicit, and to be
replaced by a maintained upstream if one ever becomes available. Includes
are spelled from the `source/` root, for example
`#include "nn/nex/Buffer.hpp"`.

## `source/nex.h` and `source/nn/pia.h` - SDK interface headers

The platform backend and the vendor internals call into Nintendo's NEX and
Pia online libraries through `#include <nex.h>` and `#include <nn/pia.h>`.
Those SDK headers are not redistributable, so the tree carries its own
declarations of the classes and functions the title actually uses, written
from the symbols the build exposes. They live at the `source/` root so the
original include spellings resolve unchanged. They declare interfaces only;
no SDK implementation is reconstructed there.

## The seam rule

A function belongs below the seam if, and only if, its call graph reaches
an operating-system import. Anything above the seam that calls a platform
function directly is a defect in the reconstruction and is fixed at the seam
rather than kept.
