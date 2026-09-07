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
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

static DoorBlock::DoorHalf gLowerDoorHalf;
static DoorBlock::DoorHalf gUpperDoorHalf;
static DoorBlock::HingeSide gLeftHingeSide;
static DoorBlock::HingeSide gRightHingeSide;

static const AABB* gDoorSouthShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.1875);
static const AABB* gDoorNorthShape = AABB::newPermanent(0.0, 0.0, 0.8125, 1.0, 1.0, 1.0);
static const AABB* gDoorWestShape = AABB::newPermanent(0.8125, 0.0, 0.0, 1.0, 1.0, 1.0);
static const AABB* gDoorEastShape = AABB::newPermanent(0.0, 0.0, 0.0, 0.1875, 1.0, 1.0);

AbstractProperty<DoorBlock::DoorHalf*>* DoorBlock::HALF = NULL;
AbstractProperty<DoorBlock::HingeSide*>* DoorBlock::HINGE = NULL;
AbstractProperty<bool>* DoorBlock::OPEN = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"open"));
AbstractProperty<bool>* DoorBlock::POWERED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"powered"));
AbstractProperty<const Direction*>* DoorBlock::FACING = NULL;
DoorBlock::DoorHalf* DoorBlock::LOWER = &gLowerDoorHalf;
DoorBlock::DoorHalf* DoorBlock::UPPER = &gUpperDoorHalf;
DoorBlock::HingeSide* DoorBlock::LEFT = &gLeftHingeSide;
DoorBlock::HingeSide* DoorBlock::RIGHT = &gRightHingeSide;

std::wstring DoorBlock::DoorHalf::toString() const { return getSerializedName(); }
std::wstring DoorBlock::DoorHalf::getSerializedName() const { return this == DoorBlock::UPPER ? L"upper" : L"lower"; }
std::wstring DoorBlock::HingeSide::toString() const { return getSerializedName(); }
std::wstring DoorBlock::HingeSide::getSerializedName() const { return this == DoorBlock::LEFT ? L"left" : L"right"; }

DoorBlock::DoorBlock(Material* material)
    : Block(material)
{
    DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    state = state->setBoxedValue(FACING, &facing);
    TypedBoxed<bool> open(false);
    state = state->setBoxedValue(OPEN, &open);
    TypedBoxed<HingeSide*> hinge(LEFT);
    state = state->setBoxedValue(HINGE, &hinge);
    TypedBoxed<bool> powered(false);
    state = state->setBoxedValue(POWERED, &powered);
    TypedBoxed<DoorHalf*> half(LOWER);
    state = state->setBoxedValue(HALF, &half);
    registerDefaultState(state);
    mUpperIcon = NULL;
    mFlippedUpperIcon = NULL;
    mLowerIcon = NULL;
    mFlippedLowerIcon = NULL;
}

DoorBlock::~DoorBlock() {}

void DoorBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING;
    boost::unordered_set<HingeSide*> hinges;
    hinges.insert(LEFT);
    hinges.insert(RIGHT);
    HINGE = EnumProperty<HingeSide*>::create(L"hinge", typeid(HingeSide*), hinges, Predicates<HingeSide*>::alwaysTrue);
    boost::unordered_set<DoorHalf*> halves;
    halves.insert(LOWER);
    halves.insert(UPPER);
    HALF = EnumProperty<DoorHalf*>::create(L"half", typeid(DoorHalf*), halves, Predicates<DoorHalf*>::alwaysTrue);
}

const AABB* DoorBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    state = state->fillVirtualBlockStateProperties(levelSource, pos);
    const Direction* facing = state->getValue(FACING);
    bool open = state->getValue(OPEN);
    bool rightHinge = state->getValue(HINGE) == RIGHT;
    switch (facing->get3DDataValue())
    {
    case 2:
        if (!open) return gDoorNorthShape;
        return rightHinge ? gDoorWestShape : gDoorEastShape;
    case 3:
        if (!open) return gDoorSouthShape;
        return rightHinge ? gDoorEastShape : gDoorWestShape;
    case 4:
        if (!open) return gDoorWestShape;
        return rightHinge ? gDoorSouthShape : gDoorNorthShape;
    default:
        if (!open) return gDoorEastShape;
        return rightHinge ? gDoorNorthShape : gDoorSouthShape;
    }
}

std::wstring DoorBlock::getName() { return L10N::GetString(getDescriptionId(-1)); }
bool DoorBlock::isSolidRender(const BlockState*) const { return false; }
bool DoorBlock::isOpenFromData(int data) { return (data >> 2) & 1; }
bool DoorBlock::isUpperFromData(int data) { return (data >> 3) & 1; }
int DoorBlock::getLowerData(int data) { return data & 7; }

int DoorBlock::getCompositeData(LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* state = levelSource->getBlockState(pos);
    int data = state->getBlock()->convertBlockStateToLegacyData(state);
    bool upper = isUpperFromData(data);
    const BlockState* lowerState = levelSource->getBlockState(pos.below());
    int lowerData = lowerState->getBlock()->convertBlockStateToLegacyData(lowerState);
    const BlockState* upperState = levelSource->getBlockState(pos.above());
    int upperData = upperState->getBlock()->convertBlockStateToLegacyData(upperState);
    if (upper)
    {
        upperData = data;
    }
    else
    {
        lowerData = data;
    }
    return getLowerData(lowerData) | (upper ? 8 : 0) | ((upperData & 1) ? 16 : 0) | ((upperData & 2) ? 32 : 0);
}

bool DoorBlock::isPathfindable(LevelSource* levelSource, const BlockPos& pos) { return isOpenFromData(getCompositeData(levelSource, pos)); }
bool DoorBlock::isCubeShaped(const BlockState*) { return false; }
int DoorBlock::getCloseSound() { return mMaterial == Material::METAL ? 0x3f3 : 0x3f4; }
int DoorBlock::getOpenSound() { return mMaterial == Material::METAL ? 0x3ed : 0x3ee; }
const MaterialColor* DoorBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    Block* block = state->getBlock();
    if (block == Blocks::iron_door) return MaterialColor::METAL;
    if (block == Blocks::wooden_door) return PlanksBlock::Variant::byData(0)->getMaterialColor();
    if (block == Blocks::spruce_door) return PlanksBlock::Variant::byData(1)->getMaterialColor();
    if (block == Blocks::birch_door) return PlanksBlock::Variant::byData(2)->getMaterialColor();
    if (block == Blocks::jungle_door) return PlanksBlock::Variant::byData(3)->getMaterialColor();
    if (block == Blocks::acacia_door) return PlanksBlock::Variant::byData(4)->getMaterialColor();
    if (block == Blocks::dark_oak_door) return PlanksBlock::Variant::byData(5)->getMaterialColor();
    return Block::getMapColor(state, levelSource, pos);
}
bool DoorBlock::TestUse() { return this != Blocks::iron_door; }

bool DoorBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (mMaterial == Material::METAL) return false;
    if (clientSide)
    {
        level->levelEvent(player, state->getValue(OPEN) ? getOpenSound() : getCloseSound(), pos, 0);
        return false;
    }
    BlockPos lowerPos = state->getValue(HALF) == LOWER ? pos : pos.below();
    if (!(pos == lowerPos)) state = level->getBlockState(lowerPos);
    if (state->getBlock() != this) return false;
    const BlockState* changedState = state->cycle(OPEN);
    level->setBlock(lowerPos, changedState, 11, false);
    level->setBlocksDirty(lowerPos, pos);
    BlockPos upperPos = lowerPos.above();
    level->updateObservingBlocksAt(upperPos, level->getBlock(upperPos));
    level->levelEvent(player, changedState->getValue(OPEN) ? getOpenSound() : getCloseSound(), pos, 0);
    return true;
}

void DoorBlock::setOpen(Level* level, const BlockPos& pos, bool open)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != this) return;
    BlockPos lowerPos = state->getValue(HALF) == UPPER ? pos.below() : pos;
    const BlockState* lowerState = state;
    if (!(pos == lowerPos)) lowerState = level->getBlockState(lowerPos);
    if (lowerState->getBlock() != this) return;
    if (lowerState->getValue(OPEN) == open) return;
    TypedBoxed<bool> value(open);
    level->setBlock(lowerPos, lowerState->setBoxedValue(OPEN, &value), 10, false);
    BlockPos upperPos = lowerPos.above();
    level->updateObservingBlocksAt(upperPos, level->getBlock(upperPos));
    level->setBlocksDirty(lowerPos, pos);
    level->levelEvent(boost::shared_ptr<Player>(), open ? getOpenSound() : getCloseSound(), pos, 0);
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
        BlockPos lowerPos = pos.below();
        const BlockState* lowerState = level->getBlockState(lowerPos);
        if (lowerState->getBlock() != this)
        {
            level->removeBlock(pos);
            return;
        }
        if (block != this) lowerState->neighborChanged(level, lowerPos, block, fromPos);
        return;
    }
    BlockPos upperPos = pos.above();
    const BlockState* upperState = level->getBlockState(upperPos);
    if (upperState->getBlock() != this)
    {
        level->removeBlock(pos);
    }
    if (!CanSurvive(level->getBlockState(pos.below())))
    {
        level->removeBlock(pos);
        if (upperState->getBlock() == this) level->removeBlock(upperPos);
        if (!level->mIsClientSide) spawnResources(level, pos, state, 0);
        return;
    }
    bool powered = level->hasNeighborSignal(pos) || level->hasNeighborSignal(pos.above());
    if (block == this || (!powered && !block->defaultBlockState()->isSignalSource())) return;
    if (powered != upperState->getValue(POWERED))
    {
        TypedBoxed<bool> value(powered);
        level->setBlock(upperPos, upperState->setBoxedValue(POWERED, &value), 3, false);
        if (powered != state->getValue(OPEN))
        {
            level->setBlock(pos, state->setBoxedValue(OPEN, &value), 3, false);
            level->setBlocksDirty(pos, pos);
            level->levelEvent(boost::shared_ptr<Player>(), powered ? getOpenSound() : getCloseSound(), pos, 0);
        }
    }
}
Item* DoorBlock::getItem()
{
    if (this == Blocks::iron_door) return Items::ironDoor;
    if (this == Blocks::spruce_door) return Items::spruceDoor;
    if (this == Blocks::birch_door) return Items::birchDoor;
    if (this == Blocks::jungle_door) return Items::jungleDoor;
    if (this == Blocks::acacia_door) return Items::acaciaDoor;
    if (this == Blocks::dark_oak_door) return Items::darkOakDoor;
    return Items::woodenDoor;
}
Item* DoorBlock::getResource(const BlockState*, Random*, int) { return getItem(); }
bool DoorBlock::mayPlace(Level* level, const BlockPos& pos) { return pos.y <= 254 && Block::mayPlace(level, pos) && Block::mayPlace(level, pos.above()) && CanSurvive(level->getBlockState(pos.below())); }
EPistonPushReaction DoorBlock::getPistonPushReaction(const BlockState*) { return PISTON_PUSH_DESTROY; }
not_null_ptr<ItemInstance> DoorBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*) { return not_null_ptr<ItemInstance>(new ItemInstance(getItem())); }
bool DoorBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    BlockPos lowerPos = pos.below();
    BlockPos upperPos = pos.above();
    if (state->getValue(HALF) == UPPER && level->getBlockState(lowerPos)->getBlock() == this)
    {
        if (player->mAbilities.mInstabuild) level->removeBlock(pos);
        level->removeBlock(lowerPos);
    }
    if (state->getValue(HALF) == LOWER && level->getBlockState(upperPos)->getBlock() == this)
    {
        if (player->mAbilities.mInstabuild) level->removeBlock(pos);
        level->removeBlock(upperPos);
    }
    return true;
}
int DoorBlock::getRenderLayer() { return 0; }
const BlockState* DoorBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (state->getValue(HALF) == LOWER)
    {
        const BlockState* upperState = levelSource->getBlockState(pos.above());
        if (upperState->getBlock() == this)
        {
            TypedBoxed<HingeSide*> hinge(upperState->getValue(HINGE));
            state = state->setBoxedValue(HINGE, &hinge);
            TypedBoxed<bool> powered(upperState->getValue(POWERED));
            state = state->setBoxedValue(POWERED, &powered);
        }
    }
    else
    {
        const BlockState* lowerState = levelSource->getBlockState(pos.below());
        if (lowerState->getBlock() == this)
        {
            TypedBoxed<const Direction*> facing(lowerState->getValue(FACING));
            state = state->setBoxedValue(FACING, &facing);
            TypedBoxed<bool> open(lowerState->getValue(OPEN));
            state = state->setBoxedValue(OPEN, &open);
        }
    }
    return state;
}
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
        state = state->setBoxedValue(OPEN, &open);
    }
    else
    {
        TypedBoxed<DoorHalf*> half(UPPER);
        TypedBoxed<HingeSide*> hinge((data & 1) != 0 ? RIGHT : LEFT);
        TypedBoxed<bool> powered((data & 2) != 0);
        state = state->setBoxedValue(HALF, &half);
        state = state->setBoxedValue(HINGE, &hinge);
        state = state->setBoxedValue(POWERED, &powered);
    }
    return state;
}
int DoorBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    if (state->getValue(HALF) == UPPER)
    {
        int data = 8;
        if (state->getValue(HINGE) == RIGHT) data |= 1;
        if (state->getValue(POWERED)) data |= 2;
        return data;
    }
    int data = state->getValue(FACING)->getClockWise()->get2DDataValue();
    if (state->getValue(OPEN)) data |= 4;
    return data;
}
unsigned int DoorBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    unsigned int tooltip = Block::GetInteractTooltip(tooltipData);
    if (mMaterial == Material::METAL)
    {
        if (!tooltipData.m_bUseItem && !tooltipData.m_bCanDestroy) return tooltip;
        if (tooltip == 0x4c36cd96 || tooltip == 0xffffffff) return 0xffffffff;
    }
    if (!tooltipData.m_bUseItem && !tooltipData.m_bCanDestroy) return tooltip;
    return 0xea4e897e;
}
const BlockState* DoorBlock::rotate(const BlockState* state, Rotation* rotation)
{
    if (state->getValue(HALF) == UPPER) return state;
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}
const BlockState* DoorBlock::mirror(const BlockState* state, Mirror* mirror)
{
    if (mirror == Mirror::NONE) return state;
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}
bool DoorBlock::isOpen(LevelSource* levelSource, const BlockPos& pos) { return isOpenFromData(getCompositeData(levelSource, pos)); }
const Direction* DoorBlock::getFacingFromData(int data) { return Direction::from2DDataValue(data & 3)->getCounterClockWise(); }
const Direction* DoorBlock::getFacing(LevelSource* levelSource, const BlockPos& pos) { return getFacingFromData(getCompositeData(levelSource, pos)); }
BlockStateDefinition* DoorBlock::createBlockStateDefinition() { const Property* properties[] = { HALF, FACING, OPEN, HINGE, POWERED }; return new BlockStateDefinition(this, properties, 5); }
EBlockFaceShape DoorBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*) { return BLOCK_FACE_UNDEFINED; }
bool DoorBlock::isDoor(Block* block)
{
    return block == Blocks::wooden_door || block == Blocks::spruce_door || block == Blocks::birch_door || block == Blocks::jungle_door || block == Blocks::acacia_door || block == Blocks::dark_oak_door;
}
TextureAtlasSprite* DoorBlock::getTexture(const Direction*, const BlockState*) { return mLowerIcon; }
TextureAtlasSprite* DoorBlock::getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP || direction == Direction::DOWN) return mLowerIcon;
    state = fillVirtualBlockStateProperties(state, levelSource, pos);
    const Direction* facing = state->getValue(FACING)->getClockWise();
    bool open = state->getValue(OPEN);
    bool flip = false;
    if (!open)
    {
        flip = (facing == Direction::SOUTH && direction == Direction::EAST) ||
               (facing == Direction::WEST && direction == Direction::SOUTH) ||
               (facing == Direction::NORTH && direction == Direction::WEST) ||
               (facing == Direction::EAST && direction == Direction::NORTH);
        if (state->getValue(HINGE) == RIGHT) flip = !flip;
    }
    else if ((facing == Direction::SOUTH && direction == Direction::NORTH) ||
             (facing == Direction::WEST && direction == Direction::EAST) ||
             (facing == Direction::NORTH && direction == Direction::SOUTH) ||
             (facing == Direction::EAST && direction == Direction::WEST))
    {
        flip = !flip;
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
    delete mFlippedUpperIcon;
    mFlippedUpperIcon = new FlippedIcon(mUpperIcon, true, false);
    delete mFlippedLowerIcon;
    mFlippedLowerIcon = new FlippedIcon(mLowerIcon, true, false);
}
EBlockRenderShape DoorBlock::getRenderShape(const BlockState*) { return BLOCK_RENDER_DOOR; }
