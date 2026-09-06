#include "game/world/block/PotatoBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include <string>

template <typename T>
std::wstring toString(T value);

static const AABB* SHAPE_BY_AGE[8];

PotatoBlock::PotatoBlock()
    : CropBlock(false)
{
}

PotatoBlock::~PotatoBlock()
{
}

Item* PotatoBlock::getBaseSeedId()
{
    return Items::potato;
}

Item* PotatoBlock::getBasePlantId()
{
    return Items::potato;
}

void PotatoBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus)
{
    CropBlock::spawnResources(level, pos, state, chance, lootBonus);
    if (level->mIsClientSide || !isMaxAge(state) || level->mRandom->nextInt(50) != 0)
    {
        return;
    }
    Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Items::potato)));
}

const AABB* PotatoBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return SHAPE_BY_AGE[state->getValue(getAgeProperty())];
}

TextureAtlasSprite* PotatoBlock::getTexture(const Direction*, const BlockState* state)
{
    int age = state->getValue(AGE);
    if (age < 7)
    {
        if (age == 6)
        {
            age = 5;
        }
        return mStageIcons[age >> 1];
    }
    return mStageIcons[3];
}

void PotatoBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 4; ++i)
    {
        mStageIcons[i] = iconRegister->registerIcon(getIconName() + L"_stage_" + ::toString(i));
    }
}
