#include "game/world/block/RepeaterBlock.hpp"
#include "game/core/StringIds.hpp"

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
#include "game/item/Items.hpp"

const double RepeaterBlock::DELAY_TORCH_OFFSETS[4] = {-0.0625, 0.0625, 0.1875, 0.3125};
AbstractProperty<bool>* RepeaterBlock::LOCKED =
    static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"locked"));
AbstractProperty<int>* RepeaterBlock::DELAY =
    static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"delay", 1, 4));
static const eGameHostOption REPEATER_INTERACTION_HOST_OPTION = eGameHostOption_MiniGame;

RepeaterBlock::RepeaterBlock(bool on) : DiodeBlock(on)
{
    DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    state = state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
    TypedBoxed<bool> locked(false);
    state = state->setBoxedValue(LOCKED, &locked);
    TypedBoxed<int> delay(1);
    registerDefaultState(state->setBoxedValue(DELAY, &delay));
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
    return state->setBoxedValue(LOCKED, &locked);
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
        if (g_pApp->GetGameHostOption(REPEATER_INTERACTION_HOST_OPTION) != 0)
            priority = 0x25;
        TypedBoxed<int> delay(state->getValue(DELAY) == 4 ? 1 : state->getValue(DELAY) + 1);
        level->setBlock(pos, state->setBoxedValue(DELAY, &delay), priority, false);
        return true;
    }
    return false;
}

int RepeaterBlock::getTurnOnDelay(const BlockState* state)
{
    return state->getValue(DELAY) << 1;
}

const BlockState* RepeaterBlock::getOnBlock(const BlockState* state)
{
    const BlockState* result = Blocks::powered_repeater->defaultBlockState();
    TypedBoxed<const Direction*> facing(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY));
    result = result->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
    TypedBoxed<int> delay(state->getValue(DELAY));
    result = result->setBoxedValue(DELAY, &delay);
    TypedBoxed<bool> locked(state->getValue(LOCKED));
    return result->setBoxedValue(LOCKED, &locked);
}

const BlockState* RepeaterBlock::getOffBlock(const BlockState* state)
{
    const BlockState* result = Blocks::unpowered_repeater->defaultBlockState();
    TypedBoxed<const Direction*> facing(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY));
    result = result->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
    TypedBoxed<int> delay(state->getValue(DELAY));
    result = result->setBoxedValue(DELAY, &delay);
    TypedBoxed<bool> locked(state->getValue(LOCKED));
    return result->setBoxedValue(LOCKED, &locked);
}

Item* RepeaterBlock::getResource(const BlockState*, Random*, int)
{
    return Items::repeater;
}

not_null_ptr<ItemInstance> RepeaterBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::repeater));
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
    if (random->nextBoolean()) offset = (float)(state->getValue(DELAY) * 2 - 1);
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
    state = state->setBoxedValue(LOCKED, &locked);
    TypedBoxed<int> delay((data >> 2) + 1);
    return state->setBoxedValue(DELAY, &delay);
}

int RepeaterBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get2DDataValue() +
           (state->getValue(DELAY) * 4 - 4);
}

unsigned int RepeaterBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseItem || tooltipData.m_bCanPlace) && tooltipData.m_bCanDestroy)
        return IDS_TOOLTIPS_USE;
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* RepeaterBlock::createBlockStateDefinition()
{
    const Property* properties[] = { HorizontalDirectionalBlock::FACING_PROPERTY, LOCKED, DELAY };
    return new BlockStateDefinition(this, properties, 3);
}

bool RepeaterBlock::TestUse()
{
    return true;
}

EBlockRenderShape RepeaterBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_REPEATER;
}
