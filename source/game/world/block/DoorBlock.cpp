#include "game/world/block/DoorBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/FlippedIcon.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<DoorBlock::DoorHalf*>* DoorBlock::HALF = NULL;
AbstractProperty<DoorBlock::HingeSide*>* DoorBlock::HINGE = NULL;
Property* DoorBlock::OPEN = NULL;
Property* DoorBlock::POWERED = NULL;
AbstractProperty<Direction*>* DoorBlock::FACING = NULL;
DoorBlock::DoorHalf* DoorBlock::LOWER = NULL;
DoorBlock::DoorHalf* DoorBlock::UPPER = NULL;
DoorBlock::HingeSide* DoorBlock::LEFT = NULL;
DoorBlock::HingeSide* DoorBlock::RIGHT = NULL;

std::wstring DoorBlock::DoorHalf::toString() const { return mName; }
std::wstring DoorBlock::DoorHalf::getSerializedName() const { return mName; }
std::wstring DoorBlock::HingeSide::toString() const { return mName; }
std::wstring DoorBlock::HingeSide::getSerializedName() const { return mName; }

DoorBlock::DoorBlock(Material* material)
    : Block(material), mUpperIcon(NULL), mFlippedUpperIcon(NULL), mLowerIcon(NULL), mFlippedLowerIcon(NULL)
{
    DerivedInit();
    TypedBoxed<DoorHalf*> half(LOWER);
    TypedBoxed<bool> open(false);
    TypedBoxed<HingeSide*> hinge(LEFT);
    TypedBoxed<bool> upper(false);
    TypedBoxed<Direction*> facing(const_cast<Direction*>(Direction::NORTH));
    const BlockState* state = mStateDefinition->any();
    if (HALF) state = state->setBoxedValue(HALF, &half);
    if (HINGE) state = state->setBoxedValue(HINGE, &hinge);
    if (OPEN) state = state->setBoxedValue(static_cast<AbstractProperty<bool>*>(OPEN), &open);
    if (POWERED) state = state->setBoxedValue(static_cast<AbstractProperty<bool>*>(POWERED), &open);
    if (FACING) state = state->setBoxedValue(FACING, &facing);
    registerDefaultState(state);
}

DoorBlock::~DoorBlock() {}

void DoorBlock::blockStaticCtor()
{
    if (LOWER == NULL) LOWER = new DoorHalf(L"lower");
    if (UPPER == NULL) UPPER = new DoorHalf(L"upper");
    if (LEFT == NULL) LEFT = new HingeSide(L"left");
    if (RIGHT == NULL) RIGHT = new HingeSide(L"right");
    boost::unordered_set<DoorHalf*> halves;
    halves.insert(LOWER); halves.insert(UPPER);
    HALF = EnumProperty<DoorHalf*>::create(L"half", typeid(DoorHalf*), halves, NULL);
    boost::unordered_set<HingeSide*> hinges;
    hinges.insert(LEFT); hinges.insert(RIGHT);
    HINGE = EnumProperty<HingeSide*>::create(L"hinge", typeid(HingeSide*), hinges, NULL);
    OPEN = BooleanProperty::create(L"open");
    POWERED = BooleanProperty::create(L"powered");
    boost::unordered_set<Direction*> directions;
    directions.insert(const_cast<Direction*>(Direction::NORTH));
    directions.insert(const_cast<Direction*>(Direction::SOUTH));
    directions.insert(const_cast<Direction*>(Direction::WEST));
    directions.insert(const_cast<Direction*>(Direction::EAST));
    FACING = EnumProperty<Direction*>::create(L"facing", typeid(Direction*), directions, NULL);
}

const AABB* DoorBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    const Direction* facing = state->getValue(FACING);
    bool open = state->getValue(static_cast<AbstractProperty<bool>*>(OPEN));
    bool rightHinge = state->getValue(HINGE) == RIGHT;
    switch (facing->get3DDataValue())
    {
    case 2:
        if (open) return rightHinge ? AABB::newPermanent(0.0, 0.0, 0.0, 0.1875, 1.0, 1.0) : AABB::newPermanent(0.8125, 0.0, 0.0, 1.0, 1.0, 1.0);
        return rightHinge ? AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.1875) : AABB::newPermanent(0.0, 0.0, 0.8125, 1.0, 1.0, 1.0);
    case 3:
        if (open) return rightHinge ? AABB::newPermanent(0.8125, 0.0, 0.0, 1.0, 1.0, 1.0) : AABB::newPermanent(0.0, 0.0, 0.0, 0.1875, 1.0, 1.0);
        return rightHinge ? AABB::newPermanent(0.0, 0.0, 0.8125, 1.0, 1.0, 1.0) : AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.1875);
    case 4:
        if (open) return rightHinge ? AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.1875) : AABB::newPermanent(0.0, 0.0, 0.8125, 1.0, 1.0, 1.0);
        return rightHinge ? AABB::newPermanent(0.0, 0.0, 0.1875, 1.0, 1.0, 1.0) : AABB::newPermanent(0.0, 0.0, 0.0, 0.1875, 1.0, 1.0);
    default:
        if (open) return rightHinge ? AABB::newPermanent(0.0, 0.0, 0.8125, 1.0, 1.0, 1.0) : AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.1875);
        return rightHinge ? AABB::newPermanent(0.0, 0.0, 0.0, 0.1875, 1.0, 1.0) : AABB::newPermanent(0.8125, 0.0, 0.0, 1.0, 1.0, 1.0);
    }
}

std::wstring DoorBlock::getName() { return L10N::GetString(getDescriptionId(-1)); }
bool DoorBlock::isSolidRender(const BlockState*) const { return false; }
bool DoorBlock::isOpenFromData(int data) { return (data >> 2) & 1; }
bool DoorBlock::isUpperFromData(int data) { return (data >> 3) & 1; }
int DoorBlock::getLowerData(int data) { return data & 7; }

int DoorBlock::getCompositeData(LevelSource* levelSource, const BlockPos& pos)
{
    int data = levelSource->getBlockData(pos.x, pos.y, pos.z);
    if (!isUpperFromData(data))
    {
        int above = levelSource->getBlockData(pos.x, pos.y + 1, pos.z);
        return getLowerData(data) | (isOpenFromData(above) ? 16 : 0) | ((above & 2) ? 32 : 0);
    }
    int below = levelSource->getBlockData(pos.x, pos.y - 1, pos.z);
    return getLowerData(below) | 8 | (data & 1 ? 16 : 0) | (data & 2 ? 32 : 0);
}

bool DoorBlock::isPathfindable(LevelSource* levelSource, const BlockPos& pos) { return isOpenFromData(getCompositeData(levelSource, pos)); }
bool DoorBlock::isCubeShaped(const BlockState*) { return false; }
int DoorBlock::getCloseSound() { return mMaterial == Material::METAL ? 0x3f3 : 0x3f4; }
int DoorBlock::getOpenSound() { return mMaterial == Material::METAL ? 0x3ed : 0x3ee; }
const MaterialColor* DoorBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    Block* block = state->getBlock();
    if (block == Blocks::wooden_door) return PlanksBlock::Variant::byData(0)->getMaterialColor();
    if (block == Blocks::spruce_door) return PlanksBlock::Variant::byData(1)->getMaterialColor();
    if (block == Blocks::birch_door) return PlanksBlock::Variant::byData(2)->getMaterialColor();
    if (block == Blocks::jungle_door) return PlanksBlock::Variant::byData(3)->getMaterialColor();
    if (block == Blocks::acacia_door) return PlanksBlock::Variant::byData(4)->getMaterialColor();
    if (block == Blocks::dark_oak_door) return PlanksBlock::Variant::byData(5)->getMaterialColor();
    return Block::getMapColor(state, levelSource, pos);
}
bool DoorBlock::TestUse() { return true; }

bool DoorBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player>, InteractionHand::EInteractionHand, const Direction*, float, float, float, bool)
{
    setOpen(level, pos, !isOpenFromData(convertBlockStateToLegacyData(state)));
    return true;
}

void DoorBlock::setOpen(Level* level, const BlockPos& pos, bool open)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != this) return;
    BlockPos lowerPos = state->getValue(HALF) == UPPER ? pos.below() : pos;
    const BlockState* lowerState = level->getBlockState(lowerPos);
    if (lowerState->getBlock() != this) return;
    if (state->getValue(static_cast<AbstractProperty<bool>*>(OPEN)) == open) return;
    TypedBoxed<bool> value(open);
    level->setBlock(lowerPos, lowerState->setBoxedValue(OPEN, &value), 10, false);
    level->updateObservingBlocksAt(lowerPos.above(), level->getBlock(lowerPos.above()));
    level->setBlocksDirty(lowerPos, pos);
    level->levelEvent(open ? getOpenSound() : getCloseSound(), pos, 0);
}

bool DoorBlock::CanSurvive(const BlockState* state)
{
    if (state->isTopSolidBlocking()) return true;
    return !Block::isExceptionForTopSolidBlocking(state->getBlock());
}
void DoorBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    if (state->getValue(HALF) == UPPER)
    {
        if (level->getBlockState(pos.below())->getBlock() != this)
        {
            level->removeBlock(pos);
            return;
        }
        Block::neighborChanged(state, level, pos, block, fromPos);
        return;
    }
    if (level->getBlockState(pos.above())->getBlock() != this)
    {
        level->removeBlock(pos);
    }
    if (!CanSurvive(state))
    {
        level->removeBlock(pos);
        if (!level->mIsClientSide) Block::neighborChanged(state, level, pos, block, fromPos);
        return;
    }
    bool powered = level->hasNeighborSignal(pos) || level->hasNeighborSignal(pos.above());
    if (powered != state->getValue(static_cast<AbstractProperty<bool>*>(POWERED)))
    {
        TypedBoxed<bool> value(powered);
        level->setBlock(pos, state->setBoxedValue(POWERED, &value), 3, false);
        level->setBlock(pos.above(), level->getBlockState(pos.above())->setBoxedValue(POWERED, &value), 3, false);
        level->setBlocksDirty(pos, pos);
        level->levelEvent(powered ? getOpenSound() : getCloseSound(), pos, 0);
    }
}
Item* DoorBlock::getItem() { return Item::byBlock(this); }
Item* DoorBlock::getResource(const BlockState*, Random*, int) { return getItem(); }
bool DoorBlock::mayPlace(Level* level, const BlockPos& pos) { return pos.y <= 254 && Block::mayPlace(level, pos) && Block::mayPlace(level, pos.above()) && CanSurvive(level->getBlockState(pos.below())); }
EPistonPushReaction DoorBlock::getPistonPushReaction(const BlockState*) { return PISTON_PUSH_DESTROY; }
not_null_ptr<ItemInstance> DoorBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*) { return not_null_ptr<ItemInstance>(new ItemInstance(getItem())); }
bool DoorBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player) { return Block::playerWillDestroy(level, pos, state, player); }
int DoorBlock::getRenderLayer() { return 2; }
const BlockState* DoorBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource*, const BlockPos&) { return state; }
const BlockState* DoorBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    if (!isUpperFromData(data))
    {
        TypedBoxed<DoorHalf*> half(LOWER);
        TypedBoxed<const Direction*> facing(getFacingFromData(data));
        TypedBoxed<bool> open((data & 4) != 0);
        state = state->setBoxedValue(HALF, &half);
        state = state->setBoxedValue(FACING, &facing);
        state = state->setBoxedValue(static_cast<AbstractProperty<bool>*>(OPEN), &open);
    }
    else
    {
        TypedBoxed<DoorHalf*> half(UPPER);
        TypedBoxed<HingeSide*> hinge((data & 1) != 0 ? RIGHT : LEFT);
        TypedBoxed<bool> powered((data & 2) != 0);
        state = state->setBoxedValue(HALF, &half);
        state = state->setBoxedValue(HINGE, &hinge);
        state = state->setBoxedValue(static_cast<AbstractProperty<bool>*>(POWERED), &powered);
    }
    return state;
}
int DoorBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    if (state->getValue(HALF) == UPPER)
    {
        int data = 8;
        if (state->getValue(HINGE) == RIGHT) data |= 1;
        if (state->getValue(static_cast<AbstractProperty<bool>*>(POWERED))) data |= 2;
        return data;
    }
    int data = state->getValue(FACING)->getClockWise()->get2DDataValue();
    if (state->getValue(static_cast<AbstractProperty<bool>*>(OPEN))) data |= 4;
    return data;
}
unsigned int DoorBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData) { return Block::GetInteractTooltip(tooltipData); }
const BlockState* DoorBlock::rotate(const BlockState* state, Rotation* rotation)
{
    if (state->getValue(HALF) == UPPER) return state;
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}
const BlockState* DoorBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}
bool DoorBlock::isOpen(LevelSource* levelSource, const BlockPos& pos) { return isOpenFromData(getCompositeData(levelSource, pos)); }
const Direction* DoorBlock::getFacingFromData(int data) { return Direction::from2DDataValue(data & 3)->getCounterClockWise(); }
const Direction* DoorBlock::getFacing(LevelSource* levelSource, const BlockPos& pos) { return getFacingFromData(getCompositeData(levelSource, pos)); }
BlockStateDefinition* DoorBlock::createBlockStateDefinition() { const Property* properties[] = { FACING, OPEN, HINGE, POWERED, HALF }; return new BlockStateDefinition(this, properties, 5); }
EBlockFaceShape DoorBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*) { return BLOCK_FACE_UNDEFINED; }
bool DoorBlock::isDoor(Block* block)
{
    return block == Blocks::wooden_door || block == Blocks::spruce_door || block == Blocks::birch_door || block == Blocks::jungle_door || block == Blocks::acacia_door || block == Blocks::dark_oak_door;
}
TextureAtlasSprite* DoorBlock::getTexture(const Direction*, const BlockState*) { return mLowerIcon; }
TextureAtlasSprite* DoorBlock::getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction, const BlockState*)
{
    if (direction == Direction::UP || direction == Direction::DOWN) return mLowerIcon;
    const BlockState* state = levelSource->getBlockState(pos);
    const Direction* facing = state->getValue(FACING)->getClockWise();
    bool open = state->getValue(static_cast<AbstractProperty<bool>*>(OPEN));
    bool flip = false;
    if (!open)
    {
        flip = (facing == Direction::NORTH && direction == Direction::EAST) ||
               (facing == Direction::SOUTH && direction == Direction::WEST) ||
               (facing == Direction::WEST && direction == Direction::NORTH) ||
               (facing == Direction::EAST && direction == Direction::SOUTH);
        if (state->getValue(HINGE) == RIGHT) flip = !flip;
    }
    if (state->getValue(HALF) == UPPER)
    {
        return flip ? mFlippedUpperIcon : mUpperIcon;
    }
    return flip ? mFlippedLowerIcon : mLowerIcon;
}
void DoorBlock::registerIcons(IconRegister* iconRegister)
{
    mUpperIcon = iconRegister->registerIcon(getIconName() + L"_upper");
    mLowerIcon = iconRegister->registerIcon(getIconName() + L"_lower");
    mFlippedUpperIcon = mUpperIcon;
    mFlippedLowerIcon = mLowerIcon;
}
EBlockRenderShape DoorBlock::getRenderShape(const BlockState*) { return BLOCK_RENDER_MODEL; }
