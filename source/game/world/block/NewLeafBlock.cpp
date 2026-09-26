#include "game/world/block/NewLeafBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/GenericStats.hpp"
#include "game/core/Stats.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/util/Random.hpp"

EnumProperty<PlanksBlock::Variant*>* NewLeafBlock::VARIANT = NULL;

bool leaf_variant_predicate(PlanksBlock::Variant* value);

bool leaf_variant_predicate(PlanksBlock::Variant* value)
{
    return value->getData() > 3;
}

void NewLeafBlock::blockStaticCtor()
{
    boost::unordered_set<PlanksBlock::Variant*> values;
    values.insert(PlanksBlock::Variant::byData(0));
    values.insert(PlanksBlock::Variant::byData(1));
    values.insert(PlanksBlock::Variant::byData(2));
    values.insert(PlanksBlock::Variant::byData(3));
    values.insert(PlanksBlock::Variant::byData(4));
    values.insert(PlanksBlock::Variant::byData(5));
    VARIANT = EnumProperty<PlanksBlock::Variant*>::create(
        L"variant", typeid(PlanksBlock::Variant*), values,
        new Predicates<PlanksBlock::Variant*>::FunctionPredicate(leaf_variant_predicate));
}

NewLeafBlock::NewLeafBlock()
    : LeafBlock()
{
    DerivedInit();
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(4));
    const BlockState* state = mStateDefinition->any()->setBoxedValue(VARIANT, &variant);
    TypedBoxed<bool> checkDecay(true);
    state = state->setBoxedValue(CHECK_DECAY, &checkDecay);
    TypedBoxed<bool> decayable(true);
    registerDefaultState(state->setBoxedValue(DECAYABLE, &decayable));
}

NewLeafBlock::~NewLeafBlock()
{
}

void NewLeafBlock::dropExtraLoot(Level* level, const BlockPos& pos, const BlockState* state, int lootBonus)
{
    if (state->getValue(VARIANT) != PlanksBlock::Variant::byData(5))
        return;
    if (level->mRandom->nextInt(lootBonus) != 0)
        return;
    Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Items::apple, 1, 0)));
}

int NewLeafBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

not_null_ptr<ItemInstance> NewLeafBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1,
        state->getBlock()->convertBlockStateToLegacyData(state) & 3));
}

not_null_ptr<ItemInstance> NewLeafBlock::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1,
        state->getValue(VARIANT)->getData() - 4));
}

const BlockState* NewLeafBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData((data & 3) + 4));
    state = state->setBoxedValue(VARIANT, &variant);
    TypedBoxed<bool> decayable(((data >> 2) & 1) == 0);
    state = state->setBoxedValue(DECAYABLE, &decayable);
    TypedBoxed<bool> checkDecay((data & 8) != 0);
    return state->setBoxedValue(CHECK_DECAY, &checkDecay);
}

int NewLeafBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData() - 4;
    if (!state->getValue(DECAYABLE))
        data |= 4;
    if (state->getValue(CHECK_DECAY))
        data |= 8;
    return data;
}

PlanksBlock::Variant* NewLeafBlock::getWoodVariantForData(int data)
{
    return PlanksBlock::Variant::byData((data & 3) + 4);
}

BlockStateDefinition* NewLeafBlock::createBlockStateDefinition()
{
    const Property* properties[3] = { VARIANT, CHECK_DECAY, DECAYABLE };
    return new BlockStateDefinition(this, properties, 3);
}

void NewLeafBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                 const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                                 not_null_ptr<ItemInstance> tool)
{
    if (!level->mIsClientSide && tool->getItem() == Items::shears)
    {
        player->awardStat(Stats::blockMinedStats.data[Block::getId(this)],
                          GenericStats::param_blocksMined(state, 1));
        Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1,
            state->getValue(VARIANT)->getData() - 4)));
        return;
    }

    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
}

int NewLeafBlock::getWoodType(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}
