#include "game/world/block/TrapDoorBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<TrapDoorBlock::Half*>* TrapDoorBlock::HALF = NULL;
AbstractProperty<bool>* TrapDoorBlock::OPEN = NULL;
AbstractProperty<const Direction*>* TrapDoorBlock::FACING = NULL;
TrapDoorBlock::Half* TrapDoorBlock::TOP = NULL;
TrapDoorBlock::Half* TrapDoorBlock::BOTTOM = NULL;

std::wstring TrapDoorBlock::Half::toString() const { return mName; }
std::wstring TrapDoorBlock::Half::getSerializedName() const { return mName; }

TrapDoorBlock::TrapDoorBlock(Material* material) : Block(material)
{
    DerivedInit();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    TypedBoxed<bool> open(false);
    TypedBoxed<Half*> half(BOTTOM);
    const BlockState* state = mStateDefinition->any();
    if (FACING != NULL) state = state->setBoxedValue(FACING, &facing);
    if (OPEN != NULL) state = state->setBoxedValue(OPEN, &open);
    if (HALF != NULL) state = state->setBoxedValue(HALF, &half);
    registerDefaultState(state);
}

TrapDoorBlock::TrapDoorBlock(Material* material, const MaterialColor* materialColor) : Block(material, materialColor)
{
    DerivedInit();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    TypedBoxed<bool> open(false);
    TypedBoxed<Half*> half(BOTTOM);
    const BlockState* state = mStateDefinition->any();
    if (FACING != NULL) state = state->setBoxedValue(FACING, &facing);
    if (OPEN != NULL) state = state->setBoxedValue(OPEN, &open);
    if (HALF != NULL) state = state->setBoxedValue(HALF, &half);
    registerDefaultState(state);
}

TrapDoorBlock::~TrapDoorBlock() {}

void TrapDoorBlock::blockStaticCtor()
{
    if (BOTTOM == NULL) BOTTOM = new Half(L"bottom");
    if (TOP == NULL) TOP = new Half(L"top");
    boost::unordered_set<Half*> halves;
    halves.insert(BOTTOM);
    halves.insert(TOP);
    HALF = EnumProperty<Half*>::create(L"half", typeid(Half*), halves, NULL);
    if (OPEN == NULL) OPEN = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"open"));
    if (FACING == NULL)
    {
        boost::unordered_set<const Direction*> directions;
        directions.insert(Direction::NORTH);
        directions.insert(Direction::SOUTH);
        directions.insert(Direction::WEST);
        directions.insert(Direction::EAST);
        FACING = EnumProperty<const Direction*>::create(L"facing", typeid(const Direction*), directions, NULL);
    }
}

const AABB* TrapDoorBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    if (state->getValue(OPEN))
    {
        const Direction* facing = state->getValue(FACING);
        switch (facing->get3DDataValue())
        {
        case 2: return AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.1875);
        case 3: return AABB::newPermanent(0.0, 0.0, 0.8125, 1.0, 1.0, 1.0);
        case 4: return AABB::newPermanent(0.0, 0.0, 0.0, 0.1875, 1.0, 1.0);
        default: return AABB::newPermanent(0.8125, 0.0, 0.0, 1.0, 1.0, 1.0);
        }
    }
    if (state->getValue(HALF) == TOP) return AABB::newPermanent(0.0, 0.8125, 0.0, 1.0, 1.0, 1.0);
    return AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.1875, 1.0);
}

bool TrapDoorBlock::isSolidRender(const BlockState*) const { return false; }
bool TrapDoorBlock::isCubeShaped(const BlockState*) { return false; }
bool TrapDoorBlock::isPathfindable(LevelSource* levelSource, const BlockPos& pos) { return !levelSource->getBlockState(pos)->getValue(OPEN); }
bool TrapDoorBlock::TestUse() { return mMaterial != Material::METAL; }

void TrapDoorBlock::playSound(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos, bool open)
{
    int sound = open ? (mMaterial == Material::METAL ? 0x40d : 0x3ef) : (mMaterial == Material::METAL ? 0x40c : 0x3f5);
    level->levelEvent(player, sound, pos, 0);
}

bool TrapDoorBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
    {
        playSound(player, level, pos, state->getValue(OPEN));
        return false;
    }
    TypedBoxed<bool> open(!state->getValue(OPEN));
    level->setBlock(pos, state->setBoxedValue(OPEN, &open), 3, false);
    playSound(player, level, pos, open.getValue());
    return true;
}

void TrapDoorBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (level->mIsClientSide) return;
    bool powered = level->hasNeighborSignal(pos);
    if (powered != state->getValue(OPEN))
    {
        TypedBoxed<bool> open(powered);
        level->setBlock(pos, state->setBoxedValue(OPEN, &open), 3, false);
        playSound(boost::shared_ptr<Player>(), level, pos, powered);
    }
}

const BlockState* TrapDoorBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float, float hitY, float, int, boost::shared_ptr<LivingEntity>)
{
    const Direction* facing = direction->getAxis()->isHorizontal() ? direction : direction->getOpposite();
    TypedBoxed<const Direction*> boxedFacing(facing);
    TypedBoxed<Half*> half(hitY > 0.5f ? TOP : BOTTOM);
    TypedBoxed<bool> open(level->hasNeighborSignal(pos));
    const BlockState* state = defaultBlockState();
    state = state->setBoxedValue(FACING, &boxedFacing);
    state = state->setBoxedValue(HALF, &half);
    return state->setBoxedValue(OPEN, &open);
}

bool TrapDoorBlock::mayPlace(Level*, const BlockPos&, const Direction*) { return true; }
const Direction* TrapDoorBlock::getFacingFromData(int data) { return Direction::from2DDataValue(data & 3); }
unsigned char TrapDoorBlock::getDataFromFacing(const Direction* direction)
{
    int value = direction->get3DDataValue();
    return value > 1 && value < 5 ? (unsigned char)(value - 2) : 3;
}
int TrapDoorBlock::getRenderLayer() { return 0; }

const BlockState* TrapDoorBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(getFacingFromData(data));
    TypedBoxed<bool> open((data & 4) != 0);
    TypedBoxed<Half*> half((data & 8) != 0 ? TOP : BOTTOM);
    const BlockState* state = defaultBlockState();
    state = state->setBoxedValue(FACING, &facing);
    state = state->setBoxedValue(OPEN, &open);
    return state->setBoxedValue(HALF, &half);
}

int TrapDoorBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = getDataFromFacing(state->getValue(FACING));
    if (state->getValue(OPEN)) data |= 4;
    if (state->getValue(HALF) == TOP) data |= 8;
    return data;
}

unsigned int TrapDoorBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bIsInside) return 0xffffffff;
    if (mMaterial == Material::METAL && tooltipData.m_pItemInstance->isEmpty()) return 0xffffffff;
    if (mMaterial != Material::METAL && (tooltipData.m_bCanPlace || tooltipData.m_bUseItem) && tooltipData.m_pItemInstance->isEmpty()) return 0xea4e897e;
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* TrapDoorBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

const BlockState* TrapDoorBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

BlockStateDefinition* TrapDoorBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FACING, OPEN, HALF };
    return new BlockStateDefinition(this, properties, 3);
}

EBlockFaceShape TrapDoorBlock::getBlockFaceShape(LevelSource*, const BlockState* state, const BlockPos&, const Direction* direction)
{
    if (!state->getValue(OPEN) &&
        ((direction == Direction::DOWN && state->getValue(HALF) == TOP) ||
         (direction == Direction::UP && state->getValue(HALF) == BOTTOM))) return BLOCK_FACE_SOLID;
    return BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape TrapDoorBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(0x2b);
}
