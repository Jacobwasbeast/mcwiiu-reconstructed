#include "game/world/block/FrostedIceBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"

static AbstractProperty<int>* AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 3));

FrostedIceBlock::FrostedIceBlock()
    : IceBlock(true)
{
    Block::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<int> age(0);
    registerDefaultState(state->setBoxedValue(AGE, &age));
    mIcon1 = NULL;
    mIcon2 = NULL;
    mIcon3 = NULL;
}

FrostedIceBlock::~FrostedIceBlock()
{
}

int FrostedIceBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(AGE);
}

const BlockState* FrostedIceBlock::getBlockState(int data)
{
    TypedBoxed<int> age(Mth::clamp(data, 0, 3));
    return defaultBlockState()->setBoxedValue(AGE, &age);
}

int FrostedIceBlock::countNeighbours(Level* level, const BlockPos& pos)
{
    int count = 0;
    for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
         it != Direction::VALUES.end(); ++it)
    {
        const BlockState* state = level->getBlockState(pos.relative(*it));
        if (state->getBlock() == this)
        {
            ++count;
            if (count > 3)
            {
                return count;
            }
        }
    }
    return count;
}

void FrostedIceBlock::slightlyMelt(Level* level, const BlockPos& pos, const BlockState* state,
                                   Random* random, bool meltNeighbors)
{
    int age = state->getValue(AGE);
    if (age < 3)
    {
        TypedBoxed<int> nextAge(age + 1);
        level->setBlock(pos, state->setBoxedValue(AGE, &nextAge), 2, false);
        level->addToTickNextTick(pos, this, Mth::nextInt(random, 20, 40));
        return;
    }

    IceBlock::melt(level, pos);
    if (meltNeighbors)
    {
        for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
             it != Direction::VALUES.end(); ++it)
        {
            BlockPos neighborPos = pos.relative(*it);
            const BlockState* neighborState = level->getBlockState(neighborPos);
            if (neighborState->getBlock() == this)
            {
                slightlyMelt(level, neighborPos, neighborState, random, false);
            }
        }
    }
}

void FrostedIceBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (random->nextInt(3) == 0 || countNeighbours(level, pos) < 4)
    {
        if (11 - state->getLightBlock() < level->getRawBrightness(pos))
        {
            slightlyMelt(level, pos, state, random, true);
            return;
        }
    }
    level->addToTickNextTick(pos, this, Mth::nextInt(random, 20, 40));
}

bool FrostedIceBlock::shouldBlockTick(Level*, const BlockPos&, const BlockState*)
{
    return true;
}

void FrostedIceBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block* block,
                                     const BlockPos&)
{
    if (block == this && countNeighbours(level, pos) < 2)
    {
        IceBlock::melt(level, pos);
    }
}

BlockStateDefinition* FrostedIceBlock::createBlockStateDefinition()
{
    const Property* properties[] = { AGE };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* FrostedIceBlock::getTexture(const Direction*, const BlockState* state)
{
    int age = state->getValue(AGE);
    if (age == 0) return mIcon;
    if (age == 1) return mIcon1;
    if (age < 3) return mIcon2;
    if (age == 3) return mIcon3;
    return mIcon;
}

void FrostedIceBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"frosted_ice_0");
    mIcon1 = iconRegister->registerIcon(L"frosted_ice_1");
    mIcon2 = iconRegister->registerIcon(L"frosted_ice_2");
    mIcon3 = iconRegister->registerIcon(L"frosted_ice_3");
}

not_null_ptr<ItemInstance> FrostedIceBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return ItemInstance::EMPTY;
}

bool FrostedIceBlock::isSilkTouchable()
{
    return false;
}
