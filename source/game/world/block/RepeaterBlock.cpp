#include "game/world/block/RepeaterBlock.hpp"

#include <typeinfo>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/client/render/ParticleType.hpp"

static AbstractProperty<int>* sDelayProperty = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"delay", 1, 4));
static AbstractProperty<bool>* sLockedProperty = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"locked"));

RepeaterBlock::RepeaterBlock(bool on) : DiodeBlock(on)
{
    ClearDerivedInit();
    DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    state = state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
    TypedBoxed<bool> locked(false);
    state = state->setBoxedValue(sLockedProperty, &locked);
    TypedBoxed<int> delay(1);
    registerDefaultState(state->setBoxedValue(sDelayProperty, &delay));
}

RepeaterBlock::~RepeaterBlock()
{
}

std::wstring RepeaterBlock::getName()
{
    return L10N::GetString(0xb97b9d5d);
}

const BlockState* RepeaterBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    TypedBoxed<bool> locked(isLocked(levelSource, pos, state));
    return state->setBoxedValue(sLockedProperty, &locked);
}

const BlockState* RepeaterBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
    return state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
}

const BlockState* RepeaterBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
}

bool RepeaterBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (!clientSide && player && !player->isSneaking())
    {
        int priority = 3;
        if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f)) != 0)
            priority = 0x25;
        TypedBoxed<int> delay(state->getValue(sDelayProperty) == 4 ? 1 : state->getValue(sDelayProperty) + 1);
        level->setBlock(pos, state->setBoxedValue(sDelayProperty, &delay), priority, false);
        return true;
    }
    return false;
}

int RepeaterBlock::getTurnOnDelay(const BlockState* state)
{
    return state->getValue(sDelayProperty) << 1;
}

const BlockState* RepeaterBlock::getOnBlock(const BlockState* state)
{
    const BlockState* result = Blocks::powered_repeater->defaultBlockState();
    TypedBoxed<const Direction*> facing(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY));
    result = result->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
    TypedBoxed<int> delay(state->getValue(sDelayProperty));
    result = result->setBoxedValue(sDelayProperty, &delay);
    TypedBoxed<bool> locked(state->getValue(sLockedProperty));
    return result->setBoxedValue(sLockedProperty, &locked);
}

const BlockState* RepeaterBlock::getOffBlock(const BlockState* state)
{
    const BlockState* result = Blocks::unpowered_repeater->defaultBlockState();
    TypedBoxed<const Direction*> facing(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY));
    result = result->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
    TypedBoxed<int> delay(state->getValue(sDelayProperty));
    result = result->setBoxedValue(sDelayProperty, &delay);
    TypedBoxed<bool> locked(state->getValue(sLockedProperty));
    return result->setBoxedValue(sLockedProperty, &locked);
}

Item* RepeaterBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::unpowered_repeater);
}

not_null_ptr<ItemInstance> RepeaterBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::unpowered_repeater)));
}

bool RepeaterBlock::isLocked(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return getAlternateSignal(levelSource, pos, state) > 0;
}

bool RepeaterBlock::isAlternateInput(const BlockState* state)
{
    return DiodeBlock::isDiode(state);
}

void RepeaterBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (!mOn)
        return;
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    double x = pos.x + 0.5 + (random->nextFloat() - 0.5) * 0.2;
    double y = pos.y + 0.4;
    double z = pos.z + 0.5 + (random->nextFloat() - 0.5) * 0.2;
    float offset = -5.0f;
    if (random->nextBoolean()) offset = (float)(state->getValue(sDelayProperty) * 2 - 1);
    x += offset * 0.0625f * facing->getNormal().x;
    z += offset * 0.0625f * facing->getNormal().z;
    level->addParticle(PARTICLE_RED_DUST, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

void RepeaterBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onRemove(level, pos, state);
    level->removeBlockEntity(pos);
}

const BlockState* RepeaterBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    TypedBoxed<const Direction*> facing(Direction::from2DDataValue(data));
    state = state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
    TypedBoxed<bool> locked(false);
    state = state->setBoxedValue(sLockedProperty, &locked);
    TypedBoxed<int> delay((data >> 2) + 1);
    return state->setBoxedValue(sDelayProperty, &delay);
}

int RepeaterBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get2DDataValue() +
           (state->getValue(sDelayProperty) * 4 - 4);
}

unsigned int RepeaterBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseItem || tooltipData.m_bCanPlace) && tooltipData.m_bCanDestroy)
        return 0xea4e897e;
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* RepeaterBlock::createBlockStateDefinition()
{
    const Property* properties[] = { HorizontalDirectionalBlock::FACING_PROPERTY, sLockedProperty, sDelayProperty };
    return new BlockStateDefinition(this, properties, 3);
}

bool RepeaterBlock::TestUse()
{
    return true;
}

EBlockRenderShape RepeaterBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(15);
}
