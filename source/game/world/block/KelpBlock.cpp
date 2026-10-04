#include "game/world/block/KelpBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/BlockId.hpp"

#include <string>

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/item/ItemId.hpp"

static const std::wstring ICON_NAME_D(L"kelp_d");
static const std::wstring ICON_NAME_C(L"kelp_c");
static const std::wstring ICON_NAME_A(L"kelp_a");
static const std::wstring ICON_NAME_B(L"kelp_b");
static const std::wstring ICON_NAME_TOP_A(L"kelp_top_a");

AbstractProperty<int>* KelpBlock::AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 15));
static const AABB* KELP_SHAPE = AABB::newPermanent(0.125, 0.0, 0.125, 0.875, 1.0, 0.875);

KelpBlock::KelpBlock()
    : Block(Material::PLANT)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(AGE, 0));
    setTicking(true);
    mIcons[0] = NULL;
    mIcons[1] = NULL;
    mIcons[2] = NULL;
    mIcons[3] = NULL;
    mIcons[4] = NULL;
}

bool KelpBlock::isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState*, bool)
{
    int top = pos.y;
    int bottom = top;
    int lower = top - 4;
    int above = top + 1;
    int below = bottom - 1;
    while (lower < below && below > 0 && level->getBlockId(pos.x, below, pos.z) == BlockId::KELP)
    {
        --lower;
        bottom = below;
        below = bottom - 1;
    }

    int upper = top + 4;
    while (above < upper && above < level->getMaxBuildHeight()
           && level->getBlockId(pos.x, above, pos.z) == BlockId::KELP)
    {
        ++upper;
        ++above;
    }

    top = above - 1;
    int growth = 14 - (top - bottom);
    return growth > 0;
}

bool KelpBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

void KelpBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState*)
{
    int top = pos.y;
    int bottom = top;
    int lower = top - 4;
    int above = top + 1;
    int below = bottom - 1;
    while (lower < below && below > 0 && level->getBlockId(pos.x, below, pos.z) == BlockId::KELP)
    {
        --lower;
        bottom = below;
        below = bottom - 1;
    }

    int upper = top + 4;
    while (above < upper && above < level->getMaxBuildHeight()
           && level->getBlockId(pos.x, above, pos.z) == BlockId::KELP)
    {
        ++upper;
        ++above;
    }
    top = above - 1;

    int growth = 14 - (top - bottom);
    if (growth <= 0)
        return;
    if (growth > 2)
        growth = 2;

    for (int y = top + 1; y <= top + growth; ++y)
    {
        BlockPos target(pos.x, y, pos.z);
        if (!mayPlace(level, target))
            return;
        level->setBlockAndData(pos.x, y, pos.z, BlockId::KELP, 0, Block::UPDATE_ALL);
    }
}

const AABB* KelpBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return KELP_SHAPE;
}

bool KelpBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool KelpBlock::isCubeShaped(const BlockState*)
{
    return false;
}

int KelpBlock::getRenderLayer()
{
    return 0;
}

EBlockFaceShape KelpBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape KelpBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_KELP;
}

void KelpBlock::checkAlive(Level* level, const BlockPos& pos)
{
    if (!canSurvive(level, pos, NULL))
    {
        Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Items::kelp)));
        level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
    }
}

int KelpBlock::getAge(const BlockState* state)
{
    return state->getValue(AGE);
}

bool KelpBlock::shouldGrow(Level* level, const BlockPos& pos)
{
    if (getAge(level->getBlockState(pos)) < 15)
    {
        const BlockState* above = level->getBlockState(pos.above());
        if (above->getMaterial() == Material::WATER)
        {
            if (above->getValue(LiquidBlock::LEVEL) == 0)
                return true;
        }
    }
    return false;
}

void KelpBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    checkAlive(level, pos);
    if (level->getBlockId(pos.x, pos.y, pos.z) != BlockId::KELP)
        return;

    if (level->isEmptyWaterBlock(pos.above()))
    {
        int height = 1;
        while (level->getBlockId(pos.x, pos.y - height, pos.z) == BlockId::KELP)
            ++height;
        if (height <= 14)
        {
            int age = level->getBlockData(pos.x, pos.y, pos.z);
            if (age == 15)
            {
                level->setBlockAndData(pos.x, pos.y + 1, pos.z, BlockId::KELP, 0, Block::UPDATE_ALL);
                level->setBlockAndData(pos.x, pos.y, pos.z, BlockId::KELP, 0, Block::UPDATE_INVISIBLE);
            }
            else
            {
                level->setBlockAndData(pos.x, pos.y, pos.z, BlockId::KELP, age + 1, Block::UPDATE_INVISIBLE);
            }
        }
    }

    if (shouldGrow(level, pos) && !level->mIsClientSide && !canSurvive(level, pos, state))
        level->addToTickNextTick(pos, this, 1);
}

void KelpBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (level->getLiquidBlock(pos)->mMaterial == mMaterial)
        level->setExtraBlock(pos, Blocks::flowing_water->defaultBlockState(), Block::UPDATE_ALL, false);
    if (level->isTouchingMaterial(pos, Material::WATER) && shouldGrow(level, pos) && !level->mIsClientSide)
        level->addToTickNextTick(pos, level->getBlock(pos), 45);
}

bool KelpBlock::mayPlace(Level* level, const BlockPos& pos)
{
    const BlockState* belowState = level->getBlockState(pos.below());
    Block* below = belowState->getBlock();
    if (below == Blocks::magma || below == Blocks::soul_sand || below == Blocks::fence
        || below == Blocks::jungle_fence || below == Blocks::nether_brick_fence || below == Blocks::spruce_fence
        || below == Blocks::birch_fence || below == Blocks::dark_oak_fence || below == Blocks::acacia_fence)
    {
        return false;
    }

    if (level->getLiquidBlock(pos)->mMaterial == Material::WATER
        && static_cast<unsigned int>(level->getLiquidBlockState(pos)->getValue(LiquidBlock::LEVEL) - 1) < 7)
    {
        return false;
    }

    if (below == Blocks::kelp && level->getLiquidBlock(pos)->mMaterial == Material::WATER)
        return true;

    if (!belowState->isSolidBlockingBlock())
    {
        if (!below->isSlabBlock())
            return false;
        if (HalfSlabBlock::isBottomSlab(below->convertBlockStateToLegacyData(belowState)))
            return false;
    }
    return level->isUnderWater(pos, 0x102, 0);
}

void KelpBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!level->mIsClientSide && !canSurvive(level, pos, state))
        level->addToTickNextTick(pos, this, 1);
}

int KelpBlock::getColor(LevelSource* levelSource, const BlockPos& pos)
{
    return levelSource->getBiome(pos)->getGrassColor(pos);
}

bool KelpBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    return mayPlace(level, pos);
}

const AABB* KelpBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

Item* KelpBlock::getResource(const BlockState*, Random*, int)
{
    return Items::kelp;
}

not_null_ptr<ItemInstance> KelpBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::kelp));
}

bool KelpBlock::waterSpreadCausesSpawn()
{
    return true;
}

bool KelpBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

bool KelpBlock::isSilkTouchable()
{
    return false;
}

const BlockState* KelpBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(AGE, data);
}

int KelpBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(AGE);
}

BlockStateDefinition* KelpBlock::createBlockStateDefinition()
{
    static const Property* properties[1] = { AGE };
    return new BlockStateDefinition(this, properties, 1);
}

unsigned int KelpBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
        && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0
        && isValidBonemealTarget((Level*)tooltipData.m_pLevel, tooltipData.m_blockPos,
                                 tooltipData.m_pBlockState, true))
        return IDS_TOOLTIPS_GROW;
    return Block::GetInteractTooltip(tooltipData);
}

std::wstring KelpBlock::getTileItemIconName()
{
    return L"kelp";
}

TextureAtlasSprite* KelpBlock::getTexture(const Direction* direction, const BlockState*)
{
    return mIcons[direction->get3DDataValue()];
}

void KelpBlock::registerIcons(IconRegister* iconRegister)
{
    mIcons[0] = iconRegister->registerIcon(ICON_NAME_D);
    mIcons[1] = iconRegister->registerIcon(ICON_NAME_C);
    mIcons[2] = iconRegister->registerIcon(ICON_NAME_A);
    mIcons[3] = iconRegister->registerIcon(ICON_NAME_B);
    mIcons[4] = iconRegister->registerIcon(ICON_NAME_TOP_A);
}
