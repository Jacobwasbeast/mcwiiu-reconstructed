#include "game/world/block/CropBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"

#include <string>

template <typename T> std::wstring toString(T value);

static const AABB* SHAPE_BY_AGE[8] =
{
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.125, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.25, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.375, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.625, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.75, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.875, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)
};
AbstractProperty<int>* CropBlock::AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 7));

CropBlock::CropBlock(bool derivedInit)
    : Bush()
{
    if (!derivedInit)
    {
        DerivedInit();
    }
    setDestroyTime(0.0f);
    setSoundType(SoundType::GRASS);
    setNotCollectStatistics();
    sendBlockData(15);
}

CropBlock::~CropBlock()
{
}

bool CropBlock::isMaxAge(const BlockState* state)
{
    return getAge(state) >= getMaxAge();
}

bool CropBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState* state, bool)
{
    return !isMaxAge(state);
}

bool CropBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

void CropBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState* state)
{
    growCrops(level, pos, state);
}

void CropBlock::DerivedInit()
{
    Block::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<int> age(0);
    registerDefaultState(state->setBoxedValue(AGE, &age));
    setTicking(true);
}

const AABB* CropBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return SHAPE_BY_AGE[getAge(state)];
}

bool CropBlock::mayPlaceOn(Level*, const BlockPos&, const BlockState* state)
{
    return state->getBlock() == Blocks::farmland;
}

AbstractProperty<int>* CropBlock::getAgeProperty()
{
    return AGE;
}

int CropBlock::getMaxAge()
{
    return 7;
}

int CropBlock::getAge(const BlockState* state)
{
    return state->getValue(getAgeProperty());
}

const BlockState* CropBlock::getStateForAge(int age)
{
    TypedBoxed<int> value(age);
    return defaultBlockState()->setBoxedValue(getAgeProperty(), &value);
}

double CropBlock::getGrowthSpeed(Block* block, Level* level, const BlockPos& pos)
{
    double speed = 1.0;
    for (int x = -1; x <= 1; ++x)
    {
        const BlockState* below = level->getBlockState(pos.below().offset(x, 0, 0));
        double contribution = 0.0;
        if (below->getBlock() == block)
        {
            contribution = below->getValue(AGE) > 0 ? 3.0 : 1.0;
        }
        if (x != 0)
        {
            contribution *= 0.25;
        }
        speed += contribution;
    }
    const BlockState* west = level->getBlockState(pos.west());
    const BlockState* east = level->getBlockState(pos.east());
    const BlockState* north = level->getBlockState(pos.north());
    const BlockState* south = level->getBlockState(pos.south());
    bool horizontal = west->getBlock() == block || east->getBlock() == block;
    bool vertical = north->getBlock() == block || south->getBlock() == block;
    if ((!horizontal || !vertical) &&
        level->getBlockState(pos.north().west())->getBlock() != block &&
        level->getBlockState(pos.south().west())->getBlock() != block &&
        level->getBlockState(pos.north().east())->getBlock() != block &&
        level->getBlockState(pos.south().east())->getBlock() != block)
    {
        speed *= 0.5;
    }
    return speed;
}

void CropBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    Bush::tick(level, pos, state, random);
    if (level->getRawBrightness(pos.above()) > 8)
    {
        int age = getAge(state);
        if (age < getMaxAge() && random->nextInt((int)(12.0 / getGrowthSpeed(this, level, pos))) == 0)
        {
            level->setBlock(pos, getStateForAge(age + 1), 2, false);
        }
    }
}

void CropBlock::growCrops(Level* level, const BlockPos& pos, const BlockState* state)
{
    int age = getAge(state) + getBonemealAgeIncrease(level);
    if (age > getMaxAge()) age = getMaxAge();
    level->setBlock(pos, getStateForAge(age), 2, false);
}

int CropBlock::getBonemealAgeIncrease(Level* level)
{
    return Mth::nextInt(level->mRandom, 2, 5);
}

bool CropBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    return (level->getDaytimeRawBrightness(pos) > 7 || level->canSeeSky(pos)) &&
        mayPlaceOn(level, pos.below(), level->getBlockState(pos.below()));
}

Item* CropBlock::getBaseSeedId()
{
    return Items::wheatSeeds;
}

Item* CropBlock::getBasePlantId()
{
    return Items::wheat;
}

void CropBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus)
{
    Block::spawnResources(level, pos, state, chance, lootBonus);
    if (level->mIsClientSide || getAge(state) < getMaxAge()) return;
    for (int i = 0; i < lootBonus + 3; ++i)
    {
        if (level->mRandom->nextInt(getMaxAge() * 2) <= getAge(state))
        {
            Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(getBaseSeedId(), 1, 0)));
        }
    }
}

Item* CropBlock::getResource(const BlockState* state, Random*, int)
{
    return isMaxAge(state) ? getBasePlantId() : getBaseSeedId();
}

not_null_ptr<ItemInstance> CropBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(getBaseSeedId()));
}

const BlockState* CropBlock::getBlockState(int data)
{
    return getStateForAge(data);
}

int CropBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return getAge(state);
}

unsigned int CropBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_pItemInstance->getItem() == Items::dye &&
        tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail &&
        DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0 &&
        isValidBonemealTarget(tooltipData.m_pLevel, tooltipData.m_blockPos,
                              tooltipData.m_pBlockState, true))
    {
        return 0x9f5fdc40u;
    }
    return Block::GetInteractTooltip(tooltipData);
}

bool CropBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

BlockStateDefinition* CropBlock::createBlockStateDefinition()
{
    const Property* properties[] = { AGE };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* CropBlock::getTexture(const Direction*, const BlockState* state)
{
    return mStageIcons[Mth::clamp(state->getValue(AGE), 0, 7)];
}

void CropBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 8; ++i)
    {
        mStageIcons[i] = iconRegister->registerIcon(L"crops_" + ::toString(i));
    }
}

EBlockRenderShape CropBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(6);
}
