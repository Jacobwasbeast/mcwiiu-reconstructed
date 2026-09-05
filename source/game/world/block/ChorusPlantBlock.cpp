#include "game/world/block/ChorusPlantBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

static AbstractProperty<bool>* CHORUS_DOWN = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"down"));
static AbstractProperty<bool>* CHORUS_UP = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"up"));
static AbstractProperty<bool>* CHORUS_NORTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"north"));
static AbstractProperty<bool>* CHORUS_EAST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"east"));
static AbstractProperty<bool>* CHORUS_SOUTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"south"));
static AbstractProperty<bool>* CHORUS_WEST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"west"));

ChorusPlantBlock::ChorusPlantBlock()
    : Block(Material::PLANT, MaterialColor::COLOR_PURPLE)
{
    DerivedInit();
}

ChorusPlantBlock::~ChorusPlantBlock()
{
}

void ChorusPlantBlock::DerivedInit()
{
    Block::DerivedInit();
    TypedBoxed<bool> down(true);
    TypedBoxed<bool> up(true);
    TypedBoxed<bool> north(true);
    TypedBoxed<bool> east(true);
    TypedBoxed<bool> south(true);
    TypedBoxed<bool> west(true);
    const BlockState* state = mStateDefinition->any();
    state = state->setBoxedValue(CHORUS_DOWN, &down);
    state = state->setBoxedValue(CHORUS_UP, &up);
    state = state->setBoxedValue(CHORUS_NORTH, &north);
    state = state->setBoxedValue(CHORUS_EAST, &east);
    state = state->setBoxedValue(CHORUS_SOUTH, &south);
    registerDefaultState(state->setBoxedValue(CHORUS_WEST, &west));
}

static bool chorusConnection(const BlockState*, const BlockState* neighbor, const ChorusPlantBlock* block)
{
    return neighbor->getBlock() == block || neighbor->getBlock() == Blocks::AIR;
}

const BlockState* ChorusPlantBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* below = levelSource->getBlockState(pos.below());
    const BlockState* above = levelSource->getBlockState(pos.above());
    const BlockState* north = levelSource->getBlockState(pos.north());
    const BlockState* east = levelSource->getBlockState(pos.east());
    const BlockState* south = levelSource->getBlockState(pos.south());
    const BlockState* west = levelSource->getBlockState(pos.west());
    TypedBoxed<bool> down(chorusConnection(state, below, this));
    TypedBoxed<bool> up(chorusConnection(state, above, this));
    TypedBoxed<bool> northValue(chorusConnection(state, north, this));
    TypedBoxed<bool> eastValue(chorusConnection(state, east, this));
    TypedBoxed<bool> southValue(chorusConnection(state, south, this));
    TypedBoxed<bool> westValue(chorusConnection(state, west, this));
    const BlockState* result = state->setBoxedValue(CHORUS_DOWN, &down);
    result = result->setBoxedValue(CHORUS_UP, &up);
    result = result->setBoxedValue(CHORUS_NORTH, &northValue);
    result = result->setBoxedValue(CHORUS_EAST, &eastValue);
    result = result->setBoxedValue(CHORUS_SOUTH, &southValue);
    return result->setBoxedValue(CHORUS_WEST, &westValue);
}

const AABB* ChorusPlantBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* actual = levelSource == NULL ? state : fillVirtualBlockStateProperties(state, levelSource, pos);
    double minX = actual->getValue(CHORUS_WEST) ? 0.0 : 0.1875;
    double minY = actual->getValue(CHORUS_DOWN) ? 0.0 : 0.1875;
    double minZ = actual->getValue(CHORUS_NORTH) ? 0.0 : 0.1875;
    double maxX = actual->getValue(CHORUS_EAST) ? 1.0 : 0.8125;
    double maxY = actual->getValue(CHORUS_UP) ? 1.0 : 0.8125;
    double maxZ = actual->getValue(CHORUS_SOUTH) ? 1.0 : 0.8125;
    return AABB::newTemp(minX, minY, minZ, maxX, maxY, maxZ);
}

void ChorusPlantBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                         std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity, bool isActualState)
{
    if (!isActualState)
        state = fillVirtualBlockStateProperties(state, level, pos);
    addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.1875, 0.1875, 0.8125, 0.8125, 0.8125));
    if (state->getValue(CHORUS_DOWN)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.0, 0.1875, 0.8125, 0.8125, 0.8125));
    if (state->getValue(CHORUS_UP)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.1875, 0.1875, 0.8125, 1.0, 0.8125));
    if (state->getValue(CHORUS_NORTH)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.1875, 0.0, 0.8125, 0.8125, 0.8125));
    if (state->getValue(CHORUS_EAST)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.1875, 0.1875, 1.0, 0.8125, 0.8125));
    if (state->getValue(CHORUS_SOUTH)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.1875, 0.1875, 0.8125, 0.8125, 1.0));
    if (state->getValue(CHORUS_WEST)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.0, 0.1875, 0.1875, 0.8125, 0.8125, 0.8125));
}

int ChorusPlantBlock::convertBlockStateToLegacyData(const BlockState*) { return 0; }

void ChorusPlantBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!canSurvive(level, pos, state)) level->blockUpdated(pos, this, true);
}

Item* ChorusPlantBlock::getResource(const BlockState*, Random*, int) { return Items::air; }
int ChorusPlantBlock::getResourceCount(Random* random) { return random->nextInt(2); }
bool ChorusPlantBlock::isCubeShaped(const BlockState*) { return false; }
bool ChorusPlantBlock::isSolidRender(const BlockState*) const { return false; }

bool ChorusPlantBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return Block::mayPlace(level, pos) && canSurvive(level, pos, NULL);
}

void ChorusPlantBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!canSurvive(level, pos, NULL)) level->blockUpdated(pos, this, true);
}

bool ChorusPlantBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    Block* above = level->getBlock(pos.above());
    Block* below = level->getBlock(pos.below());
    if (above != this && below != this)
    {
        const Direction* directions[] = { Direction::NORTH, Direction::EAST, Direction::SOUTH, Direction::WEST };
        for (int i = 0; i < 4; ++i)
            if (level->getBlock(pos.relative(directions[i])) == this) return true;
    }
    else
    {
        const Direction* directions[] = { Direction::NORTH, Direction::EAST, Direction::SOUTH, Direction::WEST };
        for (int i = 0; i < 4; ++i)
        {
            BlockPos side = pos.relative(directions[i]);
            if (level->getBlock(side) == this)
            {
                Block* sideBelow = level->getBlock(side.below());
                if (sideBelow == this || sideBelow == Blocks::end_stone) return true;
            }
        }
    }
    below = level->getBlock(pos.below());
    return below == this || below == Blocks::end_stone;
}

int ChorusPlantBlock::getRenderLayer() { return 0; }
bool ChorusPlantBlock::isPathfindable(LevelSource*, const BlockPos&) { return false; }
EBlockFaceShape ChorusPlantBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*) { return BLOCK_FACE_UNDEFINED; }
EBlockRenderShape ChorusPlantBlock::getRenderShape(const BlockState*) { return (EBlockRenderShape)46; }

const AABB* ChorusPlantBlock::getVerticalShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    if (state->getValue(CHORUS_DOWN))
        return state->getValue(CHORUS_UP) ? AABB::newTemp(0.1875, 0.1875, 0.1875, 0.8125, 0.8125, 0.8125) : AABB::newTemp(0.25, 0.0, 0.25, 0.75, 0.75, 0.75);
    if (state->getValue(CHORUS_UP)) return AABB::newTemp(0.25, 0.25, 0.25, 0.75, 1.0, 0.75);
    return NULL;
}

const AABB* ChorusPlantBlock::getHorizontalShape(const BlockState*, const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case 2: return AABB::newTemp(0.25, 0.25, 0.0, 0.75, 0.75, 0.75);
    case 3: return AABB::newTemp(0.25, 0.25, 0.25, 1.0, 0.75, 0.75);
    case 4: return AABB::newTemp(0.25, 0.25, 0.25, 0.75, 0.75, 1.0);
    case 5: return AABB::newTemp(0.0, 0.25, 0.25, 0.75, 0.75, 0.75);
    default: return NULL;
    }
}

const AABB* ChorusPlantBlock::getRandomNoduleShape(bool large, const BlockState*, const Direction* direction)
{
    double low = large ? 0.3125 : 0.25;
    double high = large ? 0.8125 : 0.75;
    double far = large ? 0.875 : 0.8125;
    switch (direction->get3DDataValue())
    {
    case 2: return AABB::newTemp(low, low, 0.0, high, high, low);
    case 3: return AABB::newTemp(low, low, low, far, high, high);
    case 4: return AABB::newTemp(low, low, low, high, high, far);
    case 5: return AABB::newTemp(0.0, low, low, low, high, high);
    default: return NULL;
    }
}

bool ChorusPlantBlock::isWaterBlocking() const { return false; }
bool ChorusPlantBlock::waterSpreadCausesSpawn() { return true; }
bool ChorusPlantBlock::canContainLiquid(const BlockState*) const { return false; }
bool ChorusPlantBlock::canProvideSupport(const BlockState*, const Direction*, EBlockSupportType) { return false; }

BlockStateDefinition* ChorusPlantBlock::createBlockStateDefinition()
{
    const Property* properties[] = { CHORUS_DOWN, CHORUS_UP, CHORUS_NORTH, CHORUS_EAST, CHORUS_SOUTH, CHORUS_WEST };
    return new BlockStateDefinition(this, properties, 6);
}
