#include "game/world/block/OldLeafBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/FoliageColor.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

bool oldleaf_filter(PlanksBlock::Variant* value)
{
    return value->getData() < 4;
}

EnumProperty<PlanksBlock::Variant*>* OldLeafBlock::VARIANT = NULL;
Property* OldLeafBlock_VARIANT = NULL;

void OldLeafBlock::blockStaticCtor()
{
    boost::unordered_set<PlanksBlock::Variant*> values;
    values.insert(PlanksBlock::Variant::OAK);
    values.insert(PlanksBlock::Variant::SPRUCE);
    values.insert(PlanksBlock::Variant::BIRCH);
    values.insert(PlanksBlock::Variant::JUNGLE);
    values.insert(PlanksBlock::Variant::ACACIA);
    values.insert(PlanksBlock::Variant::DARK_OAK);
    VARIANT = EnumProperty<PlanksBlock::Variant*>::create(
        L"variant", typeid(PlanksBlock::Variant*), values,
        new Predicates<PlanksBlock::Variant*>::FunctionPredicate(oldleaf_filter));
    OldLeafBlock_VARIANT = VARIANT;
}

OldLeafBlock::OldLeafBlock()
    : LeafBlock()
{
    DerivedInit();
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(0));
    const BlockState* state = mStateDefinition->any()->setBoxedValue(VARIANT, &variant);
    TypedBoxed<bool> decayable(true);
    state = state->setBoxedValue(DECAYABLE, &decayable);
    TypedBoxed<bool> checkDecay(true);
    registerDefaultState(state->setBoxedValue(CHECK_DECAY, &checkDecay));
}

OldLeafBlock::~OldLeafBlock() {}

int OldLeafBlock::getColor(const BlockState* state)
{
    PlanksBlock::Variant* variant = state->getValue(VARIANT);
    if (variant == PlanksBlock::Variant::byData(1)) return FoliageColor::getEvergreenColor();
    if (variant == PlanksBlock::Variant::byData(2)) return FoliageColor::getBirchColor();
    return LeafBlock::getColor(state);
}

int OldLeafBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex)
{
    const BlockState* state = levelSource->getBlockState(pos);
    if (state != NULL && state->getBlock() == this) return getColor(state);
    return LeafBlock::getColor(levelSource, pos, tintIndex);
}

void OldLeafBlock::dropExtraLoot(Level* level, const BlockPos& pos, const BlockState* state, int lootBonus)
{
    if (state->getValue(VARIANT) != PlanksBlock::Variant::byData(3)) return;
    if (level->mRandom->nextInt(lootBonus) != 0) return;
    Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Items::apple, 1, 0)));
}

int OldLeafBlock::getLootDropChance(const BlockState* state)
{
    if (state->getValue(VARIANT) == PlanksBlock::Variant::byData(3)) return 40;
    return LeafBlock::getLootDropChance(state);
}

not_null_ptr<ItemInstance> OldLeafBlock::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1,
        state->getValue(VARIANT)->getData()));
}

const BlockState* OldLeafBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(data & 3));
    state = state->setBoxedValue(VARIANT, &variant);
    TypedBoxed<bool> decayable((data & 4) == 0);
    state = state->setBoxedValue(DECAYABLE, &decayable);
    TypedBoxed<bool> checkDecay((data & 8) != 0);
    return state->setBoxedValue(CHECK_DECAY, &checkDecay);
}

int OldLeafBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    if (!state->getValue(DECAYABLE)) data |= 4;
    if (state->getValue(CHECK_DECAY)) data |= 8;
    return data;
}

PlanksBlock::Variant* OldLeafBlock::getWoodVariantForData(int data)
{
    return PlanksBlock::Variant::byData(data & 3);
}

BlockStateDefinition* OldLeafBlock::createBlockStateDefinition()
{
    const Property* properties[3] = { VARIANT, CHECK_DECAY, DECAYABLE };
    return new BlockStateDefinition(this, properties, 3);
}

int OldLeafBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

void OldLeafBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                  const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                                  not_null_ptr<ItemInstance> tool)
{
    if (level->mIsClientSide || tool->getItem() != Items::shears)
    {
        Block::playerDestroy(level, player, pos, state, blockEntity, tool);
        return;
    }
    player->awardStat(gStats->get_blocksMined(Block::getId(this)), GenericStats::param_blocksMined(state, 1));
    Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1,
        state->getValue(VARIANT)->getData())));
}

int OldLeafBlock::getWoodType(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}
