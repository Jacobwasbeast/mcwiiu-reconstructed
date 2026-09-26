#include "game/world/block/StemBlock.hpp"
#include "game/core/StringIds.hpp"

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
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TorchBlock.hpp"
#include "game/item/ItemId.hpp"

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
AbstractProperty<int>* StemBlock::AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 7));
DirectionProperty* StemBlock::FACING = NULL;

StemBlock::StemBlock(Block* fruitType)
    : Bush(),
      mAngledTexture(NULL),
      mFruitType(fruitType)
{
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
    return state->getValue(AGE) != 7;
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
    FACING = TorchBlock::FACING;
}

const AABB* StemBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return STEM_SHAPES[state->getValue(AGE)];
}

const BlockState* StemBlock::fillVirtualBlockStateProperties(const BlockState* state,
                                                              LevelSource* levelSource,
                                                              const BlockPos& pos)
{
    int age = state->getValue(AGE);
    TypedBoxed<const Direction*> up(Direction::UP);
    state = state->setBoxedValue(FACING, &up);
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        if (levelSource->getBlockState(pos.relative(directions.data[i]))->getBlock() == mFruitType && age == 7)
        {
            TypedBoxed<const Direction*> facing(directions.data[i]);
            return state->setBoxedValue(FACING, &facing);
        }
    }
    return state;
}

bool StemBlock::mayPlaceOn(Level*, const BlockPos&, const BlockState* state)
{
    return state->getBlock() == Blocks::farmland;
}

void StemBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    BlockPos stemPos = pos;
    Block::tick(level, stemPos, state, random);
    if (level->getRawBrightness(stemPos.above()) <= 8)
        return;

    int chance = static_cast<int>(25.0f / CropBlock::getGrowthSpeed(this, level, stemPos));
    if (random->nextInt(chance + 1) != 0)
        return;

    int age = state->getValue(AGE);
    if (age < 7)
    {
        TypedBoxed<int> nextAge(age + 1);
        level->setBlock(stemPos, state->setBoxedValue(AGE, &nextAge), Block::UPDATE_CLIENTS, false);
        return;
    }

    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        if (level->getBlockState(stemPos.relative(directions.data[i]))->getBlock() == mFruitType)
            return;
    }

    stemPos = stemPos.relative(Direction::Plane::HORIZONTAL->getRandomDirection(*random));
    Block* below = level->getBlockState(stemPos.below())->getBlock();
    if (level->getBlockState(stemPos)->getBlock()->mMaterial == Material::AIR
        && (below == Blocks::farmland || below == Blocks::dirt || below == Blocks::grass))
    {
        level->setBlockAndUpdate(stemPos, mFruitType->defaultBlockState());
    }
}

void StemBlock::growCrops(Level* level, const BlockPos& pos, const BlockState* state)
{
    int age = state->getValue(AGE) + Mth::nextInt(level->mRandom, 2, 5);
    if (age > 7)
        age = 7;
    TypedBoxed<int> nextAge(age);
    level->setBlock(pos, state->setBoxedValue(AGE, &nextAge), Block::UPDATE_CLIENTS, false);
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
    int age = state->getValue(AGE);
    for (int i = 0; i < 3; ++i)
    {
        if (level->mRandom->nextInt(15) <= age)
            Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(getSeedItem())));
    }
}

Item* StemBlock::getResource(const BlockState*, Random*, int)
{
    return Items::air;
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
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE &&
        tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail &&
        DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0 &&
        isValidBonemealTarget(tooltipData.m_pLevel, tooltipData.m_blockPos,
                              tooltipData.m_pBlockState, true))
        return IDS_TOOLTIPS_GROW;
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
    mAngledTexture = iconRegister->registerIcon(L"stem_bent");
}

TextureAtlasSprite* StemBlock::getAngledTexture()
{
    return mAngledTexture;
}

EBlockRenderShape StemBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_STEM;
}
