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

static inline AABB* createChorusShape(double minX, double minY, double minZ, double maxX, double maxY, double maxZ)
{
    AABB* shape = AABB::newPermanent(minX, minY, minZ, maxX, maxY, maxZ);
    shape->divideInternalsBy(16.0);
    return shape;
}

static inline AABB* createTempChorusShape(double minX, double minY, double minZ, double maxX, double maxY, double maxZ)
{
    AABB* shape = AABB::newTemp(minX, minY, minZ, maxX, maxY, maxZ);
    shape->divideInternalsBy(16.0);
    return shape;
}

static const AABB* CHORUS_NORTH_SHAPE = createChorusShape(4.0, 4.0, 0.0, 12.0, 12.0, 4.0);
static const AABB* CHORUS_SOUTH_SHAPE = createChorusShape(4.0, 4.0, 12.0, 12.0, 12.0, 16.0);
static const AABB* CHORUS_EAST_SHAPE = createChorusShape(12.0, 4.0, 4.0, 16.0, 12.0, 12.0);
static const AABB* CHORUS_WEST_SHAPE = createChorusShape(0.0, 4.0, 4.0, 4.0, 12.0, 12.0);

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
    const BlockState* actual = levelSource == NULL ? state : state->fillVirtualBlockStateProperties(levelSource, pos);
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
        state = state->fillVirtualBlockStateProperties(level, pos);
    addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.1875, 0.1875, 0.8125, 0.8125, 0.8125));
    if (state->getValue(CHORUS_WEST)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.0, 0.1875, 0.1875, 0.1875, 0.8125, 0.8125));
    if (state->getValue(CHORUS_EAST)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.8125, 0.1875, 0.1875, 1.0, 0.8125, 0.8125));
    if (state->getValue(CHORUS_UP)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.8125, 0.1875, 0.8125, 1.0, 0.8125));
    if (state->getValue(CHORUS_DOWN)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.0, 0.1875, 0.8125, 0.1875, 0.8125));
    if (state->getValue(CHORUS_NORTH)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.1875, 0.0, 0.8125, 0.8125, 0.1875));
    if (state->getValue(CHORUS_SOUTH)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.1875, 0.1875, 0.8125, 0.8125, 0.8125, 1.0));
}

int ChorusPlantBlock::convertBlockStateToLegacyData(const BlockState*) { return 0; }

void ChorusPlantBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!canSurvive(level, pos, state)) level->destroyBlock(pos, true);
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
    if (!canSurvive(level, pos, NULL)) level->addToTickNextTick(pos, this, 1);
}

bool ChorusPlantBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    bool aboveEmpty = level->isEmptyBlock(pos.above());
    bool belowEmpty = level->isEmptyBlock(pos.below());

    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        BlockPos side = pos.relative(directions.data[i]);
        if (level->getBlock(side) == this)
        {
            if (!aboveEmpty && !belowEmpty)
            {
                return false;
            }
            Block* sideBelow = level->getBlock(side.below());
            if (sideBelow == this || sideBelow == Blocks::end_stone)
            {
                return true;
            }
        }
    }

    Block* below = level->getBlock(pos.below());
    return below == this || below == Blocks::end_stone;
}

int ChorusPlantBlock::getRenderLayer() { return 0; }
bool ChorusPlantBlock::isPathfindable(LevelSource*, const BlockPos&) { return false; }
EBlockFaceShape ChorusPlantBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*) { return BLOCK_FACE_UNDEFINED; }
EBlockRenderShape ChorusPlantBlock::getRenderShape(const BlockState*) { return (EBlockRenderShape)46; }

const AABB* ChorusPlantBlock::getVerticalShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    if (state->getValue(CHORUS_DOWN))
    {
        if (state->getValue(CHORUS_UP))
        {
            return createTempChorusShape(4.0, 0.0, 4.0, 12.0, 16.0, 12.0);
        }
        return createTempChorusShape(4.0, 0.0, 4.0, 12.0, 12.0, 12.0);
    }
    if (state->getValue(CHORUS_UP))
    {
        return createTempChorusShape(4.0, 4.0, 4.0, 12.0, 16.0, 12.0);
    }
    return NULL;
}

const AABB* ChorusPlantBlock::getHorizontalShape(const BlockState*, const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case Direction::eDirection_North: return CHORUS_NORTH_SHAPE;
    case Direction::eDirection_South: return CHORUS_SOUTH_SHAPE;
    case Direction::eDirection_West: return CHORUS_WEST_SHAPE;
    case Direction::eDirection_East: return CHORUS_EAST_SHAPE;
    default: return NULL;
    }
}

const AABB* ChorusPlantBlock::getRandomNoduleShape(bool large, const BlockState*, const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case Direction::eDirection_North:
        return large ? createTempChorusShape(5.0, 5.0, 2.0, 11.0, 11.0, 4.0)
                     : createTempChorusShape(4.0, 4.0, 3.0, 12.0, 12.0, 4.0);
    case Direction::eDirection_South:
        return large ? createTempChorusShape(5.0, 5.0, 11.0, 11.0, 11.0, 14.0)
                     : createTempChorusShape(4.0, 4.0, 12.0, 12.0, 12.0, 13.0);
    case Direction::eDirection_West:
        return large ? createTempChorusShape(2.0, 5.0, 5.0, 4.0, 11.0, 11.0)
                     : createTempChorusShape(3.0, 4.0, 4.0, 4.0, 12.0, 12.0);
    case Direction::eDirection_East:
        return large ? createTempChorusShape(12.0, 5.0, 5.0, 14.0, 11.0, 11.0)
                     : createTempChorusShape(12.0, 4.0, 4.0, 13.0, 12.0, 12.0);
    default:
        return NULL;
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
