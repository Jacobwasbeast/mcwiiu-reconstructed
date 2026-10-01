#include "game/world/block/NetherWartBlock.hpp"

#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include <string>

template <typename T>
std::wstring toString(T value);

static AbstractProperty<int>* AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 3));
static const AABB* SHAPE_BY_AGE[4] = {
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.375, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.625, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.875, 1.0)
};

NetherWartBlock::NetherWartBlock()
    : Bush(Material::PLANT, MaterialColor::PLANT)
{
    DerivedInit();
    TypedBoxed<int> age(0);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(AGE, &age));
    setTicking(true);
}

const AABB* NetherWartBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return SHAPE_BY_AGE[state->getValue(AGE)];
}

bool NetherWartBlock::mayPlaceOn(Level*, const BlockPos&, const BlockState* state)
{
    return state->getBlock() == Blocks::soul_sand;
}

bool NetherWartBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    return mayPlaceOn(level, pos.below(), level->getBlockState(pos.below()));
}

void NetherWartBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    int age = state->getValue(AGE);
    if (age < 3 && random->nextInt(10) == 0)
    {
        TypedBoxed<int> nextAge(age + 1);
        state = state->setBoxedValue(AGE, &nextAge);
        level->setBlock(pos, state, Block::UPDATE_CLIENTS, false);
    }
    Bush::tick(level, pos, state, random);
}

void NetherWartBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float, int lootBonus)
{
    if (level->mIsClientSide)
        return;

    int count = 1;
    if (state->getValue(AGE) > 2)
    {
        count = level->mRandom->nextInt(3) + 2;
        if (lootBonus > 0)
            count += level->mRandom->nextInt(lootBonus + 1);
    }

    while (count > 0)
    {
        Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Items::netherWart)));
        --count;
    }
}

Item* NetherWartBlock::getResource(const BlockState*, Random*, int)
{
    return Items::air;
}

int NetherWartBlock::getResourceCount(Random*)
{
    return 0;
}

not_null_ptr<ItemInstance> NetherWartBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::netherWart));
}

const BlockState* NetherWartBlock::getBlockState(int data)
{
    TypedBoxed<int> age(data);
    return defaultBlockState()->setBoxedValue(AGE, &age);
}

int NetherWartBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(AGE);
}

bool NetherWartBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

BlockStateDefinition* NetherWartBlock::createBlockStateDefinition()
{
    const Property* properties[] = { AGE };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* NetherWartBlock::getTexture(const Direction*, const BlockState* state)
{
    int age = state->getValue(AGE);
    if (age > 2)
        return mStageIcons[2];
    if (age > 0)
        return mStageIcons[1];
    return mStageIcons[0];
}

void NetherWartBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 3; ++i)
        mStageIcons[i] = iconRegister->registerIcon(getIconName() + L"_stage_" + ::toString(i));
}

EBlockRenderShape NetherWartBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_ROW;
}
