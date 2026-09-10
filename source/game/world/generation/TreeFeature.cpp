#include "game/world/generation/TreeFeature.hpp"

#include <cstdlib>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

extern CMinecraftApp* g_pApp;
extern Property* OldLogBlock_VARIANT;
extern Property* OldLeafBlock_VARIANT;
extern Property* OldLeafBlock_DECAYABLE;

static const BlockState* treeTrunkState = NULL;
static const BlockState* treeLeafState = NULL;
static Property* treeCocoaAge = NULL;
static Property* treeCocoaFacing = NULL;
static const BooleanProperty* treeVineWest = NULL;
static const BooleanProperty* treeVineNorth = NULL;
static const BooleanProperty* treeVineSouth = NULL;
static const BooleanProperty* treeVineEast = NULL;

TreeFeature::TreeFeature(bool doBlockNotify)
    : AbstractTreeFeature(doBlockNotify, 4, treeTrunkState, treeLeafState, false)
{
}

TreeFeature::TreeFeature(bool doBlockNotify, int baseHeight,
                         const BlockState* trunkState,
                         const BlockState* leafState, bool hasVines)
    : AbstractTreeFeature(doBlockNotify, baseHeight, trunkState, leafState, hasVines)
{
}

TreeFeature::~TreeFeature()
{
}

void TreeFeature::staticCtor()
{
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(0));
    treeTrunkState = Blocks::log->defaultBlockState()->setBoxedValue(
        OldLogBlock_VARIANT, &variant);
    treeLeafState = Blocks::leaves->defaultBlockState()->setBoxedValue(
        OldLeafBlock_VARIANT, &variant);
    TypedBoxed<bool> decayable(false);
    treeLeafState = treeLeafState->setBoxedValue(
        OldLeafBlock_DECAYABLE, &decayable);
}

int TreeFeature::prepareSpawn(Level* level, const BlockPos& pos, int height)
{
    PIXBeginNamedEvent(0.0f, "TreeFeature::prepareSpawn");
    bool free = true;
    if (pos.y > 0 && pos.y + height + 1 < 257)
    {
        LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
        if (options != NULL && options->checkIntersects(pos.offset(-2, -1, -2),
                                                         pos.offset(2, height, 2)))
        {
            free = false;
        }
        MutableBlockPos check;
        for (int y = pos.y; free && y <= pos.y + height + 1; ++y)
        {
            int radius = y == pos.y ? 0 : (y >= pos.y + height - 1 ? 2 : 1);
            for (int x = pos.x - radius; free && x <= pos.x + radius; ++x)
            {
                for (int z = pos.z - radius; z <= pos.z + radius; ++z)
                {
                    check.set(x, y, z);
                    if (!isFree(level->getBlock(check)))
                    {
                        free = false;
                        break;
                    }
                }
            }
        }
        Block* ground = level->getBlock(pos.below());
        if (free && (ground == Blocks::grass || ground == Blocks::dirt ||
                     ground == Blocks::mycelium) &&
            pos.y < 255 - height)
        {
            setDirtAt(level, pos.below());
            PIXEndNamedEvent();
            return 1;
        }
    }
    PIXEndNamedEvent();
    return 0;
}

void TreeFeature::placeLeaf(Level* level, const MutableBlockPos& pos)
{
    Block* block = level->getBlock(pos);
    if (block == Blocks::AIR || block == Blocks::leaves || block == Blocks::leaves2)
    {
        setBlock(level, pos, mLeafState, 0);
    }
}

void TreeFeature::addHangingVine(Level* level, const BlockPos& pos,
                                 const BooleanProperty* property)
{
    BlockPos vinePos = pos;
    addVine(level, vinePos, property);
    for (int i = 4; i > 0; --i)
    {
        vinePos = vinePos.below();
        if (level->getBlock(vinePos) != Blocks::AIR)
        {
            break;
        }
        addVine(level, vinePos, property);
    }
}

void TreeFeature::placeCocoa(Level* level, int age, const BlockPos& pos,
                             const Direction* direction)
{
    TypedBoxed<int> ageValue(age);
    const BlockState* state = Blocks::cocoa->defaultBlockState();
    state = state->setBoxedValue(treeCocoaAge, &ageValue);
    TypedBoxed<const Direction*> directionValue(direction);
    state = state->setBoxedValue(treeCocoaFacing, &directionValue);
    setBlock(level, pos, state, 0);
}

bool TreeFeature::place(Level* level, Random& random, const BlockPos& pos)
{
    PIXBeginNamedEvent(0.0f, "TreeFeature::place");
    const int height = random.nextInt(3) + mBaseHeight;
    if (!prepareSpawn(level, pos, height) ||
        !placeTrunk(level, pos, random, height))
    {
        PIXEndNamedEvent();
        return false;
    }

    PIXBeginNamedEvent(0.0f, "Placing TreeFeature leaves");
    MutableBlockPos leafPos;
    for (int y = pos.y + height; y >= pos.y + height - 3; --y)
    {
        const int layer = pos.y + height - y;
        const int radius = 1 - layer / 2;
        for (int x = pos.x - radius; x <= pos.x + radius; ++x)
        {
            for (int z = pos.z - radius; z <= pos.z + radius; ++z)
            {
                if (std::abs(x - pos.x) != radius || std::abs(z - pos.z) != radius ||
                    radius <= 0 || random.nextInt(2) != 0)
                {
                    leafPos.set(x, y, z);
                    placeLeaf(level, leafPos);
                }
            }
        }
    }
    PIXEndNamedEvent();

    if (mHasVines)
    {
        PIXBeginNamedEvent(0.0f, "TreeFeature adding vines");
        for (int y = pos.y + height - 3; y <= pos.y + height; ++y)
        {
            const int distanceFromTop = pos.y + height - y;
            const int radius = 2 + (distanceFromTop + 1) / 2;
            for (int x = pos.x - radius; x <= pos.x + radius; ++x)
            {
                for (int z = pos.z - radius; z <= pos.z + radius; ++z)
                {
                    BlockPos vinePos(x, y, z);
                    if (level->getBlock(vinePos) == Blocks::leaves)
                    {
                        BlockPos west = vinePos.west();
                        if (random.nextInt(4) == 0 && level->getBlock(west) == Blocks::AIR)
                            addHangingVine(level, west, treeVineEast);
                        BlockPos east = vinePos.east();
                        if (random.nextInt(4) == 0 && level->getBlock(east) == Blocks::AIR)
                            addHangingVine(level, east, treeVineWest);
                        BlockPos north = vinePos.north();
                        if (random.nextInt(4) == 0 && level->getBlock(north) == Blocks::AIR)
                            addHangingVine(level, north, treeVineSouth);
                        BlockPos south = vinePos.south();
                        if (random.nextInt(4) == 0 && level->getBlock(south) == Blocks::AIR)
                            addHangingVine(level, south, treeVineNorth);
                    }
                }
            }
        }
        PIXEndNamedEvent();

        if (random.nextInt(5) == 0 && height > 5)
        {
            PIXBeginNamedEvent(0.0f, "TreeFeature adding cocoa");
            arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
            for (int y = 0; y < 2; ++y)
            {
                const int cocoaY = pos.y + height - 5 + y;
                for (unsigned int i = 0; i < directions.length; ++i)
                {
                    const Direction* direction = directions.data[i];
                    if (random.nextInt(4 - y) == 0)
                    {
                        const Direction* opposite = direction->getOpposite();
                        placeCocoa(level, random.nextInt(3),
                                   pos.offset(opposite->getNormal().x, cocoaY - pos.y,
                                              opposite->getNormal().z), direction);
                    }
                }
            }
            PIXEndNamedEvent();
        }
    }
    PIXEndNamedEvent();
    return true;
}
