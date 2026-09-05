#include "game/world/block/FenceBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/LeadItem.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

static AbstractProperty<bool>* FENCE_NORTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"north"));
static AbstractProperty<bool>* FENCE_EAST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"east"));
static AbstractProperty<bool>* FENCE_SOUTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"south"));
static AbstractProperty<bool>* FENCE_WEST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"west"));

FenceBlock::FenceBlock(const std::wstring& name, Material* material, const MaterialColor* materialColor)
    : Block(material, materialColor), mTextureName(name)
{
    _init();
}

FenceBlock::~FenceBlock()
{
}

void FenceBlock::_init()
{
    mBlockType = EBLOCK_TYPE_FENCE;
    DerivedInit();
    TypedBoxed<bool> falseValue(false);
    const BlockState* state = mStateDefinition->any();
    state = state->setBoxedValue(FENCE_NORTH, &falseValue);
    state = state->setBoxedValue(FENCE_EAST, &falseValue);
    state = state->setBoxedValue(FENCE_SOUTH, &falseValue);
    registerDefaultState(state->setBoxedValue(FENCE_WEST, &falseValue));
}

unsigned int FenceBlock::getAABBIndex(const BlockState* state)
{
    unsigned int result = 0;
    if (state->getValue(FENCE_SOUTH)) result |= 1u << Direction::SOUTH->get2DDataValue();
    if (state->getValue(FENCE_WEST)) result |= 1u << Direction::WEST->get2DDataValue();
    if (state->getValue(FENCE_NORTH)) result |= 1u << Direction::NORTH->get2DDataValue();
    if (state->getValue(FENCE_EAST)) result |= 1u << Direction::EAST->get2DDataValue();
    return result;
}

const AABB* FenceBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* actual = levelSource == NULL ? state : fillVirtualBlockStateProperties(state, levelSource, pos);
    double minX = actual->getValue(FENCE_WEST) ? 0.0 : 0.375;
    double minZ = actual->getValue(FENCE_NORTH) ? 0.0 : 0.375;
    double maxX = actual->getValue(FENCE_EAST) ? 1.0 : 0.625;
    double maxZ = actual->getValue(FENCE_SOUTH) ? 1.0 : 0.625;
    return AABB::newTemp(minX, 0.0, minZ, maxX, 1.0, maxZ);
}

void FenceBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                    std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity, bool isActualState)
{
    if (!isActualState) state = fillVirtualBlockStateProperties(state, level, pos);
    addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.375, 0.0, 0.375, 0.625, 1.0, 0.625));
    if (state->getValue(FENCE_NORTH)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.375, 0.0, 0.0, 0.625, 1.0, 0.625));
    if (state->getValue(FENCE_EAST)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.375, 0.0, 0.375, 1.0, 1.0, 0.625));
    if (state->getValue(FENCE_SOUTH)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.375, 0.0, 0.375, 0.625, 1.0, 1.0));
    if (state->getValue(FENCE_WEST)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.0, 0.0, 0.375, 0.625, 1.0, 0.625));
}

bool FenceBlock::isSolidRender(const BlockState*) const { return false; }
bool FenceBlock::isCubeShaped(const BlockState*) { return false; }
bool FenceBlock::isPathfindable(LevelSource*, const BlockPos&) { return false; }
bool FenceBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*) { return true; }
int FenceBlock::convertBlockStateToLegacyData(const BlockState*) { return 0; }
bool FenceBlock::canContainLiquid(const BlockState*) const { return true; }

bool FenceBlock::isExceptionBlockForAttachingPlusPiston(Block* block)
{
    return Block::isExceptionBlockForAttachingPlusPiston(block);
}

bool FenceBlock::connectsTo(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    Block* block = levelSource->getBlock(pos.relative(direction));
    if (block == this) return true;
    if (block != NULL && block->mBlockType == EBLOCK_TYPE_FENCE) return true;
    return block != NULL && !isExceptionBlockForAttachingPlusPiston(block) && block->getRenderLayer() == 5;
}

bool FenceBlock::canProvideSupport(const BlockState*, const Direction* direction, EBlockSupportType supportType)
{
    return direction == Direction::UP && (supportType == BLOCK_SUPPORT_CENTER || supportType == BLOCK_SUPPORT_ANY);
}

bool FenceBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (!clientSide) return LeadItem::bindPlayerMobs(player, level, pos);
    return LeadItem::bindPlayerMobsTest(player, level, pos);
}

const BlockState* FenceBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    TypedBoxed<bool> north(connectsTo(levelSource, pos, Direction::NORTH));
    TypedBoxed<bool> east(connectsTo(levelSource, pos, Direction::EAST));
    TypedBoxed<bool> south(connectsTo(levelSource, pos, Direction::SOUTH));
    TypedBoxed<bool> west(connectsTo(levelSource, pos, Direction::WEST));
    const BlockState* result = state->setBoxedValue(FENCE_NORTH, &north);
    result = result->setBoxedValue(FENCE_EAST, &east);
    result = result->setBoxedValue(FENCE_SOUTH, &south);
    return result->setBoxedValue(FENCE_WEST, &west);
}

const BlockState* FenceBlock::rotate(const BlockState* state, Rotation* rotation)
{
    bool north = state->getValue(FENCE_NORTH);
    bool east = state->getValue(FENCE_EAST);
    bool south = state->getValue(FENCE_SOUTH);
    bool west = state->getValue(FENCE_WEST);
    TypedBoxed<bool> valueNorth(north);
    TypedBoxed<bool> valueEast(east);
    TypedBoxed<bool> valueSouth(south);
    TypedBoxed<bool> valueWest(west);
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90:
        state = state->setBoxedValue(FENCE_NORTH, &valueWest);
        state = state->setBoxedValue(FENCE_EAST, &valueNorth);
        state = state->setBoxedValue(FENCE_SOUTH, &valueEast);
        return state->setBoxedValue(FENCE_WEST, &valueSouth);
    case Rotation::eTYPE_Clockwise180:
        state = state->setBoxedValue(FENCE_NORTH, &valueSouth);
        state = state->setBoxedValue(FENCE_EAST, &valueWest);
        state = state->setBoxedValue(FENCE_SOUTH, &valueNorth);
        return state->setBoxedValue(FENCE_WEST, &valueEast);
    case Rotation::eTYPE_CounterClockwise90:
        state = state->setBoxedValue(FENCE_NORTH, &valueEast);
        state = state->setBoxedValue(FENCE_EAST, &valueSouth);
        state = state->setBoxedValue(FENCE_SOUTH, &valueWest);
        return state->setBoxedValue(FENCE_WEST, &valueNorth);
    default: return state;
    }
}

const BlockState* FenceBlock::mirror(const BlockState* state, Mirror* mirror)
{
    if (mirror->getType() == Mirror::eTYPE_LeftRight)
    {
        bool north = state->getValue(FENCE_NORTH);
        bool south = state->getValue(FENCE_SOUTH);
        TypedBoxed<bool> valueNorth(south);
        TypedBoxed<bool> valueSouth(north);
        state = state->setBoxedValue(FENCE_NORTH, &valueNorth);
        return state->setBoxedValue(FENCE_SOUTH, &valueSouth);
    }
    if (mirror->getType() == Mirror::eTYPE_FrontBack)
    {
        bool east = state->getValue(FENCE_EAST);
        bool west = state->getValue(FENCE_WEST);
        TypedBoxed<bool> valueEast(west);
        TypedBoxed<bool> valueWest(east);
        state = state->setBoxedValue(FENCE_EAST, &valueEast);
        return state->setBoxedValue(FENCE_WEST, &valueWest);
    }
    return Block::mirror(state, mirror);
}

BlockStateDefinition* FenceBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FENCE_NORTH, FENCE_EAST, FENCE_SOUTH, FENCE_WEST };
    return new BlockStateDefinition(this, properties, 4);
}

EBlockFaceShape FenceBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction* direction)
{
    return direction == Direction::UP || direction == Direction::DOWN ? BLOCK_FACE_CENTER : BLOCK_FACE_MIDDLE_POLE_THICK;
}

void FenceBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mTextureName);
}

EBlockRenderShape FenceBlock::getRenderShape(const BlockState*) { return (EBlockRenderShape)11; }

unsigned int FenceBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    boost::shared_ptr<Player> player = (boost::shared_ptr<Player>&)tooltipData.m_pPlayer;
    return LeadItem::bindPlayerMobsTest(player, (Level*)tooltipData.m_pLevel, tooltipData.m_blockPos)
        ? 0x7b938f36u : Block::GetInteractTooltip(tooltipData);
}
