#include "game/world/block/StemBlock.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CropBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DirectionalBlock.hpp"

#include <string>

static const AABB* STEM_SHAPES[8] =
{
    AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.125, 0.625),
    AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.25, 0.625),
    AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.375, 0.625),
    AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.5, 0.625),
    AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.625, 0.625),
    AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.75, 0.625),
    AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 0.875, 0.625),
    AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 1.0, 0.625)
};
AbstractProperty<int>* StemBlock::AGE = NULL;
DirectionProperty* StemBlock::FACING = NULL;

StemBlock::StemBlock(Block* fruitType)
    : Bush(),
      mAngledTexture(NULL),
      mFruitType(fruitType)
{
    if (AGE == NULL)
        AGE = CropBlock::AGE;
    if (FACING == NULL)
        FACING = DirectionalBlock::FACING;
    DerivedInit();
    TypedBoxed<int> age(0);
    TypedBoxed<const Direction*> facing(Direction::UP);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(AGE, &age)
                         ->setBoxedValue(FACING, &facing));
    setTicking(true);
}

StemBlock::~StemBlock()
{
}

bool StemBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState* state, bool)
{
    return state->getValue(CropBlock::AGE) != 7;
}

bool StemBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

void StemBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState* state)
{
    growCrops(level, pos, state);
}

void StemBlock::blockStaticCtor()
{
    AGE = CropBlock::AGE;
    FACING = DirectionalBlock::FACING;
}

const AABB* StemBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return STEM_SHAPES[state->getValue(AGE)];
}

const BlockState* StemBlock::fillVirtualBlockStateProperties(const BlockState* state,
                                                              LevelSource* levelSource,
                                                              const BlockPos& pos)
{
    TypedBoxed<const Direction*> defaultFacing(Direction::UP);
    const BlockState* result = state->setBoxedValue(FACING, &defaultFacing);

    if (state->getValue(AGE) == 7)
    {
        const Direction* horizontalDirections[] = { Direction::NORTH, Direction::SOUTH,
                                                    Direction::WEST, Direction::EAST };
        for (int i = 0; i < 4; ++i)
        {
            BlockPos neighbourPos = pos.relative(horizontalDirections[i]);
            if (levelSource->getBlockState(neighbourPos)->getBlock() == mFruitType)
            {
                TypedBoxed<const Direction*> facing(horizontalDirections[i]);
                return state->setBoxedValue(FACING, &facing);
            }
        }
    }
    return result;
}

bool StemBlock::mayPlaceOn(Level*, const BlockPos&, const BlockState* state)
{
    return state->getBlock() == Blocks::farmland;
}

void StemBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    Block::tick(level, pos, state, random);
    if (level->getRawBrightness(pos.above()) <= 8)
        return;

    int age = state->getValue(AGE);
    if (age < 7 && random->nextInt((int)(25.0 / CropBlock::getGrowthSpeed(this, level, pos))) == 0)
    {
        TypedBoxed<int> nextAge(age + 1);
        level->setBlock(pos, state->setBoxedValue(CropBlock::AGE, &nextAge), 2, false);
        return;
    }

    if (age != 7 || random->nextInt((int)(25.0 / CropBlock::getGrowthSpeed(this, level, pos))) != 0)
        return;

    const Direction* directions[] = { Direction::NORTH, Direction::SOUTH,
                                      Direction::WEST, Direction::EAST };
    for (int i = 0; i < 4; ++i)
        if (level->getBlockState(pos.relative(directions[i]))->getBlock() == mFruitType)
            return;

    const Direction* direction = Direction::Plane::HORIZONTAL->getRandomDirection(*random);
    BlockPos target = pos.relative(direction);
    const BlockState* below = level->getBlockState(target.below());
    const BlockState* targetState = level->getBlockState(target);
    if (targetState->getBlock() == Blocks::AIR &&
        (below->getBlock() == Blocks::farmland || below->getBlock() == Blocks::dirt ||
         below->getBlock() == Blocks::grass))
        level->setBlock(target, mFruitType->defaultBlockState(), 2, false);
}

void StemBlock::growCrops(Level* level, const BlockPos& pos, const BlockState* state)
{
    int age = state->getValue(AGE) + Mth::nextInt(level->mRandom, 2, 5);
    if (age > 7)
        age = 7;
    TypedBoxed<int> nextAge(age);
    level->setBlock(pos, state->setBoxedValue(AGE, &nextAge), 2, false);
}

int StemBlock::getColor(const BlockState* state)
{
    if (state->getBlock() != this)
        return Block::getColor(state);
    int age = state->getValue(AGE);
    return (age << 21) | (0xff00 - (age << 11)) | (age << 2);
}

int StemBlock::getColor(LevelSource* levelSource, const BlockPos& pos)
{
    return getColor(levelSource, pos, levelSource->getBlockState(pos));
}

int StemBlock::getColor(LevelSource*, const BlockPos&, const BlockState* state)
{
    return getColor(state);
}

bool StemBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

Item* StemBlock::getSeedItem()
{
    if (mFruitType == Blocks::pumpkin)
        return Items::pumpkinSeeds;
    if (mFruitType == Blocks::melon_block)
        return Items::melonSeeds;
    return NULL;
}

void StemBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state,
                               float chance, int lootBonus)
{
    Block::spawnResources(level, pos, state, chance, lootBonus);
    if (level->mIsClientSide || getSeedItem() == NULL)
        return;
    int age = state->getValue(CropBlock::AGE);
    for (int i = 0; i < 3; ++i)
    {
        if (level->mRandom->nextInt(15) <= age)
            Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(getSeedItem())));
    }
}

Item* StemBlock::getResource(const BlockState*, Random*, int)
{
    return Items::melonSeeds;
}

not_null_ptr<ItemInstance> StemBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(getSeedItem()));
}

const BlockState* StemBlock::getBlockState(int data)
{
    TypedBoxed<int> age(data);
    return defaultBlockState()->setBoxedValue(AGE, &age);
}

int StemBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(AGE);
}

unsigned int StemBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == 351 &&
        tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail &&
        DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0 &&
        isValidBonemealTarget(tooltipData.m_pLevel, tooltipData.m_blockPos,
                              tooltipData.m_pBlockState, true))
        return 0x9f5fdc40U;
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* StemBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FACING, AGE };
    return new BlockStateDefinition(this, properties, 2);
}

void StemBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"stem_straight");
    mAngledTexture = iconRegister->registerIcon(L"stem_angled");
}

TextureAtlasSprite* StemBlock::getAngledTexture()
{
    return mAngledTexture;
}

EBlockRenderShape StemBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(19);
}
