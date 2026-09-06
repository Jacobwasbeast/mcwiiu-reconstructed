#include "game/world/block/BeetrootBlock.hpp"

#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include <string>

template <typename T>
std::wstring toString(T value);

static const AABB* SHAPE_BY_AGE[4];

BeetrootBlock::BeetrootBlock()
    : CropBlock(true)
{
    DerivedInit();
    for (int i = 0; i < 4; ++i)
        mStageIcons[i] = 0;
}

BeetrootBlock::~BeetrootBlock()
{
}

AbstractProperty<int>* BeetrootBlock::getAgeProperty()
{
    return AGE;
}

int BeetrootBlock::getMaxAge()
{
    return 3;
}

Item* BeetrootBlock::getBaseSeedId()
{
    return Items::beetrootSeeds;
}

Item* BeetrootBlock::getBasePlantId()
{
    return Items::beetroot;
}

void BeetrootBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (random->nextInt(3) == 0)
    {
        checkAlive(level, pos, state);
        return;
    }
    CropBlock::tick(level, pos, state, random);
}

int BeetrootBlock::getBonemealAgeIncrease(Level* level)
{
    return CropBlock::getBonemealAgeIncrease(level) / 3;
}

BlockStateDefinition* BeetrootBlock::createBlockStateDefinition()
{
    const Property* properties[] = { AGE };
    return new BlockStateDefinition(this, properties, 1);
}

const AABB* BeetrootBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return SHAPE_BY_AGE[state->getValue(getAgeProperty())];
}

TextureAtlasSprite* BeetrootBlock::getTexture(const Direction*, const BlockState* state)
{
    int age = Mth::clamp(state->getValue(AGE), 0, getMaxAge());
    return mStageIcons[age];
}

void BeetrootBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i <= getMaxAge(); ++i)
        mStageIcons[i] = iconRegister->registerIcon(L"beetroots_stage_" + ::toString(i));
}
