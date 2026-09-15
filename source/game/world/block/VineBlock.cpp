#include "game/world/block/VineBlock.hpp"

#include "game/core/FoliageColor.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

BooleanProperty* VineBlock::UP = static_cast<BooleanProperty*>(BooleanProperty::create(L"up"));
BooleanProperty* VineBlock::NORTH = static_cast<BooleanProperty*>(BooleanProperty::create(L"north"));
BooleanProperty* VineBlock::SOUTH = static_cast<BooleanProperty*>(BooleanProperty::create(L"south"));
BooleanProperty* VineBlock::WEST = static_cast<BooleanProperty*>(BooleanProperty::create(L"west"));
BooleanProperty* VineBlock::EAST = static_cast<BooleanProperty*>(BooleanProperty::create(L"east"));

static const AABB* VINE_UP = AABB::newPermanent(0.0, 0.9375, 0.0, 1.0, 1.0, 1.0);
static const AABB* VINE_NORTH = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.0625);
static const AABB* VINE_SOUTH = AABB::newPermanent(0.0, 0.0, 0.9375, 1.0, 1.0, 1.0);
static const AABB* VINE_WEST = AABB::newPermanent(0.0, 0.0, 0.0, 0.0625, 1.0, 1.0);
static const AABB* VINE_EAST = AABB::newPermanent(0.9375, 0.0, 0.0, 1.0, 1.0, 1.0);

VineBlock::VineBlock() : Block(Material::PLANT)
{
    DerivedInit();
    bool falseValue = false;
    TypedBoxed<bool> boxed(falseValue);
    const BlockState* state = defaultBlockState();
    state = state->setBoxedValue(UP, &boxed);
    state = state->setBoxedValue(NORTH, &boxed);
    state = state->setBoxedValue(SOUTH, &boxed);
    state = state->setBoxedValue(WEST, &boxed);
    state = state->setBoxedValue(EAST, &boxed);
    registerDefaultState(state);
    setTicking(true);
}

VineBlock::~VineBlock() {}

const AABB* VineBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&) { return NULL; }

const AABB* VineBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    if (countFaces(state) != 1) return NULL;
    if (state->getValue(UP)) return VINE_UP;
    if (state->getValue(NORTH)) return VINE_NORTH;
    if (state->getValue(SOUTH)) return VINE_SOUTH;
    if (state->getValue(WEST)) return VINE_WEST;
    return VINE_EAST;
}

const BlockState* VineBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    bool up = !levelSource->getBlockState(pos.above())->getBlock()->isSolidFace(levelSource, pos.above(), Direction::DOWN);
    TypedBoxed<bool> value(up);
    return state->setBoxedValue(UP, &value);
}

bool VineBlock::isSolidRender(const BlockState*) const { return false; }
bool VineBlock::isCubeShaped(const BlockState*) { return false; }
bool VineBlock::mayReplaceWithPlace(LevelSource*, const BlockPos&) { return true; }

bool VineBlock::isExceptionBlockForAttaching(Block* block)
{
    return block == NULL || dynamic_cast<VineBlock*>(block) != NULL || block == Blocks::AIR ||
           block == Blocks::WATER || block == Blocks::FLOWING_WATER || block == Blocks::lava ||
           block == Blocks::flowing_lava || block == Blocks::FIRE || block == Blocks::snow_layer ||
           block == Blocks::redstone_wire || block == Blocks::torch || block == Blocks::redstone_torch ||
           block == Blocks::tripwire || block == Blocks::deadbush || block == Blocks::tallgrass;
}

bool VineBlock::isAcceptableNeighbor(Level* level, const BlockPos& pos, const Direction* direction)
{
    Block* block = level->getBlockState(pos)->getBlock();
    if (!block->isSolidFace(level, pos, direction)) return !isExceptionBlockForAttaching(block);
    return false;
}

bool VineBlock::canAttachBelow(Block* block)
{
    return block == Blocks::leaves || block == Blocks::leaves2 || block == Blocks::log ||
           block == Blocks::log2 || block == Blocks::vine || block == Blocks::cocoa;
}

bool VineBlock::canAttachTo(Level* level, const BlockPos& pos, const Direction* direction)
{
    const BlockPos above = pos.above();
    Block* aboveBlock = level->getBlockState(above)->getBlock();
    if (!isAcceptableNeighbor(level, pos.relative(direction->getOpposite()), direction)) return false;
    return canAttachBelow(aboveBlock) || isAcceptableNeighbor(level, above, Direction::DOWN);
}

bool VineBlock::mayPlace(Level* level, const BlockPos& pos, const Direction* direction)
{
    return direction != Direction::UP && direction != Direction::DOWN && canAttachTo(level, pos, direction);
}

AbstractProperty<bool>* VineBlock::getPropertyForFace(const Direction* direction)
{
    switch (direction->get3DDataValue())
    {
    case Direction::eDirection_Up: return UP;
    case Direction::eDirection_North: return NORTH;
    case Direction::eDirection_South: return SOUTH;
    case Direction::eDirection_West: return WEST;
    case Direction::eDirection_East: return EAST;
    default: return NULL;
    }
}

unsigned char VineBlock::countFaces(const BlockState* state)
{
    return (unsigned char)(state->getValue(UP) + state->getValue(NORTH) + state->getValue(SOUTH) + state->getValue(WEST) + state->getValue(EAST));
}

bool VineBlock::updateSurvival(Level* level, const BlockPos& pos, const BlockState* state)
{
    const BlockState* newState = state;
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; i++)
    {
        const Direction* direction = directions.data[i];
        AbstractProperty<bool>* property = getPropertyForFace(direction);
        if (newState->getValue(property))
        {
            if (!canAttachTo(level, pos, direction->getOpposite()))
            {
                const BlockState* aboveState = level->getBlockState(pos.above());
                if (aboveState->getBlock() == this && aboveState->getValue(property))
                {
                    continue;
                }
                TypedBoxed<bool> value(false);
                newState = newState->setBoxedValue(property, &value);
            }
        }
    }
    if (countFaces(newState) == 0) return false;
    if (state != newState) level->setBlock(pos, newState, 2, false);
    return true;
}

int VineBlock::getColor() const { return FoliageColor::getDefaultColor(); }
int VineBlock::getColor(const BlockState*) { return FoliageColor::getDefaultColor(); }
int VineBlock::getColor(LevelSource*, const BlockPos&, const BlockState*) { return getColor(); }
int VineBlock::getColor(LevelSource* levelSource, const BlockPos& pos) { return levelSource->getBlockState(pos)->getBlock()->getColor(levelSource, pos, levelSource->getBlockState(pos)); }

void VineBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!level->mIsClientSide && !updateSurvival(level, pos, state))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
}

void VineBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (level->mIsClientSide || random->nextInt(4) != 0) return;
    const Direction* direction = Direction::getRandomDirection(*random);
    AbstractProperty<bool>* property = getPropertyForFace(direction);
    if (property == NULL || state->getValue(property)) return;
    if (!canAttachTo(level, pos, direction)) return;
    TypedBoxed<bool> value(true);
    level->setBlock(pos, state->setBoxedValue(property, &value), 2, false);
}

const BlockState* VineBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float, float, float, int, boost::shared_ptr<LivingEntity>)
{
    if (!mayPlace(level, pos, direction)) return NULL;
    TypedBoxed<bool> value(true);
    return defaultBlockState()->setBoxedValue(getPropertyForFace(direction), &value);
}

Item* VineBlock::getResource(const BlockState*, Random*, int) { return Items::shears; }
int VineBlock::getResourceCount(Random*) { return 0; }

void VineBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos, const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity, not_null_ptr<ItemInstance> tool)
{
    if (!level->mIsClientSide && tool->getItem() == Items::shears)
    {
        popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Blocks::vine, 1, 0)));
        return;
    }
    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
}

int VineBlock::getRenderLayer() { return 0; }

const BlockState* VineBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    bool up = (data & 1) != 0;
    bool north = (data & 2) != 0;
    bool south = (data & 4) != 0;
    bool west = (data & 8) != 0;
    bool east = (data & 16) != 0;
    TypedBoxed<bool> value(up); state = state->setBoxedValue(UP, &value);
    value = TypedBoxed<bool>(north); state = state->setBoxedValue(NORTH, &value);
    value = TypedBoxed<bool>(south); state = state->setBoxedValue(SOUTH, &value);
    value = TypedBoxed<bool>(west); state = state->setBoxedValue(WEST, &value);
    value = TypedBoxed<bool>(east); return state->setBoxedValue(EAST, &value);
}

int VineBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return (state->getValue(UP) ? 1 : 0) | (state->getValue(NORTH) ? 2 : 0) | (state->getValue(SOUTH) ? 4 : 0) | (state->getValue(WEST) ? 8 : 0) | (state->getValue(EAST) ? 16 : 0);
}

BlockStateDefinition* VineBlock::createBlockStateDefinition()
{
    const Property* properties[] = { UP, NORTH, SOUTH, WEST, EAST };
    return new BlockStateDefinition(this, properties, 5);
}

const BlockState* VineBlock::rotate(const BlockState* state, Rotation* rotation)
{
    bool north = state->getValue(NORTH);
    bool south = state->getValue(SOUTH);
    bool west = state->getValue(WEST);
    bool east = state->getValue(EAST);
    if (rotation->getType() == Rotation::eTYPE_Clockwise90)
    {
        TypedBoxed<bool> value(north);
        const BlockState* result = state->setBoxedValue(EAST, &value);
        value = TypedBoxed<bool>(east);
        result = result->setBoxedValue(SOUTH, &value);
        value = TypedBoxed<bool>(south);
        result = result->setBoxedValue(WEST, &value);
        value = TypedBoxed<bool>(west);
        return result->setBoxedValue(NORTH, &value);
    }
    if (rotation->getType() == Rotation::eTYPE_Clockwise180)
    {
        TypedBoxed<bool> value(west);
        const BlockState* result = state->setBoxedValue(EAST, &value);
        value = TypedBoxed<bool>(north);
        result = result->setBoxedValue(SOUTH, &value);
        value = TypedBoxed<bool>(east);
        result = result->setBoxedValue(WEST, &value);
        value = TypedBoxed<bool>(south);
        return result->setBoxedValue(NORTH, &value);
    }
    if (rotation->getType() == Rotation::eTYPE_CounterClockwise90)
    {
        TypedBoxed<bool> value(south);
        const BlockState* result = state->setBoxedValue(EAST, &value);
        value = TypedBoxed<bool>(west);
        result = result->setBoxedValue(SOUTH, &value);
        value = TypedBoxed<bool>(north);
        result = result->setBoxedValue(WEST, &value);
        value = TypedBoxed<bool>(east);
        return result->setBoxedValue(NORTH, &value);
    }
    return state;
}

const BlockState* VineBlock::mirror(const BlockState* state, Mirror* mirror)
{
    if (mirror->getType() == Mirror::eTYPE_LeftRight)
    {
        bool west = state->getValue(WEST);
        bool east = state->getValue(EAST);
        TypedBoxed<bool> value(west);
        const BlockState* result = state->setBoxedValue(EAST, &value);
        value = TypedBoxed<bool>(east);
        return result->setBoxedValue(WEST, &value);
    }
    if (mirror->getType() == Mirror::eTYPE_FrontBack)
    {
        bool north = state->getValue(NORTH);
        bool south = state->getValue(SOUTH);
        TypedBoxed<bool> value(north);
        const BlockState* result = state->setBoxedValue(SOUTH, &value);
        value = TypedBoxed<bool>(south);
        return result->setBoxedValue(NORTH, &value);
    }
    return Block::mirror(state, mirror);
}
EBlockFaceShape VineBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*) { return (EBlockFaceShape)8; }
EBlockRenderShape VineBlock::getRenderShape(const BlockState*) { return (EBlockRenderShape)20; }
bool VineBlock::shouldBlockTick(Level*, const BlockPos&, const BlockState*) { return !Minecraft::InMiniGame((EMiniGameId)15, true); }
bool VineBlock::waterSpreadCausesSpawn() { return true; }
