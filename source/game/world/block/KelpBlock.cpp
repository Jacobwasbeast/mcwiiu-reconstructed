#include "game/world/block/KelpBlock.hpp"

#include <string>

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
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

static AbstractProperty<int>* KELP_AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 15));

KelpBlock::KelpBlock()
    : Block(Material::PLANT)
{
    DerivedInit();
    TypedBoxed<int> age(0);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(KELP_AGE, &age));
    setTicking(true);
    mIcons[0] = NULL;
    mIcons[1] = NULL;
    mIcons[2] = NULL;
    mIcons[3] = NULL;
    mIcons[4] = NULL;
}

KelpBlock::~KelpBlock()
{
}

bool KelpBlock::isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState*, bool)
{
    int bottom = pos.y;
    int lower = bottom - 4;
    while (lower < bottom - 1 && bottom - 1 > 0
           && level->getBlockId(pos.x, bottom - 1, pos.z) == 258)
    {
        --lower;
        --bottom;
    }

    int upper = pos.y + 4;
    int top = pos.y;
    while (top + 1 < upper && top + 1 < level->getMaxBuildHeight()
           && level->getBlockId(pos.x, top + 1, pos.z) == 258)
    {
        ++upper;
        ++top;
    }

    unsigned int distance = 14U - static_cast<unsigned int>(top - bottom);
    return ((-distance & ~distance) >> 31) != 0;
}

bool KelpBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

void KelpBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState*)
{
    int bottom = pos.y;
    int lower = bottom - 4;
    while (lower < bottom - 1 && bottom - 1 > 0
           && level->getBlockId(pos.x, bottom - 1, pos.z) == 258)
    {
        --lower;
        --bottom;
    }

    int upper = pos.y + 4;
    int top = pos.y;
    while (top + 1 < upper && top + 1 < level->getMaxBuildHeight()
           && level->getBlockId(pos.x, top + 1, pos.z) == 258)
    {
        ++upper;
        ++top;
    }

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
        level->setBlockAndData(pos.x, y, pos.z, 258, 0, 3);
    }
}

const AABB* KelpBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
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
    return static_cast<EBlockRenderShape>(49);
}

void KelpBlock::checkAlive(Level* level, const BlockPos& pos)
{
    if (!canSurvive(level, pos, NULL))
    {
        Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Items::kelp)));
        level->setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
    }
}

int KelpBlock::getAge(const BlockState* state)
{
    return state->getValue(KELP_AGE);
}

bool KelpBlock::shouldGrow(Level* level, const BlockPos& pos)
{
    if (getAge(level->getBlockState(pos)) >= 15)
        return false;
    const BlockState* above = level->getBlockState(pos.above());
    return above->getMaterial() == Material::WATER && above->getValue(KELP_AGE) == 0;
}

void KelpBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    checkAlive(level, pos);
    if (level->getBlockId(pos.x, pos.y, pos.z) != 258)
        return;

    if (level->isEmptyWaterBlock(pos.above()))
    {
        int height = 1;
        while (level->getBlockId(pos.x, pos.y - height, pos.z) == 258)
            ++height;
        if (height <= 14)
        {
            int age = level->getBlockData(pos.x, pos.y, pos.z);
            if (age == 15)
            {
                level->setBlockAndData(pos.x, pos.y + 1, pos.z, 258, 0, 3);
                level->setBlockAndData(pos.x, pos.y, pos.z, 258, 0, 4);
            }
            else
            {
                level->setBlockAndData(pos.x, pos.y, pos.z, 258, age + 1, 4);
            }
        }
    }

    if (shouldGrow(level, pos) && !level->mIsClientSide && !canSurvive(level, pos, state))
        level->blockUpdated(pos, this, true);
}

void KelpBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (level->getBlockState(pos)->getMaterial() == mMaterial)
        level->setExtraBlock(pos, Blocks::WATER->defaultBlockState(), 3, false);
    if (level->isTouchingMaterial(pos, Material::WATER) && shouldGrow(level, pos)
        && !level->mIsClientSide)
        level->levelEvent(0x2D, pos, level->getBlockState(pos)->getBlock()->getColor());
}

bool KelpBlock::mayPlace(Level* level, const BlockPos& pos)
{
    const BlockState* below = level->getBlockState(pos.below());
    const Material* material = below->getMaterial();
    if (material == Material::AIR || material == Material::WATER || material == Material::LAVA
        || material == Material::FIRE || material == Material::PORTAL || material == Material::WEB
        || material == Material::PLANT || material == Material::REPLACEABLE_PLANT)
        return false;
    if (below->getBlock() == this && getAge(below) < 7)
        return false;
    if (!below->isSolid() && !below->getBlock()->isSolidRender(below))
        return false;
    return level->isUnderWater(pos, 258, 0);
}

void KelpBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!level->mIsClientSide && !canSurvive(level, pos, state))
        level->blockUpdated(pos, this, true);
}

int KelpBlock::getColor(LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* state = levelSource->getBlockState(pos);
    return state->getMapColor(levelSource, pos)->calculateRGBColor(0);
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
    TypedBoxed<int> age(data);
    return defaultBlockState()->setBoxedValue(KELP_AGE, &age);
}

int KelpBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(KELP_AGE);
}

BlockStateDefinition* KelpBlock::createBlockStateDefinition()
{
    const Property* properties[1] = { KELP_AGE };
    return new BlockStateDefinition(this, properties, 1);
}

unsigned int KelpBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == 351
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
        && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0
        && isValidBonemealTarget((Level*)tooltipData.m_pLevel, tooltipData.m_blockPos,
                                 tooltipData.m_pBlockState, true))
        return 0x9F5FDC40U;
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
    mIcons[0] = iconRegister->registerIcon(L"kelp_top");
    mIcons[1] = iconRegister->registerIcon(L"kelp_bottom");
    mIcons[2] = iconRegister->registerIcon(L"kelp_side");
    mIcons[3] = iconRegister->registerIcon(L"kelp_side");
    mIcons[4] = iconRegister->registerIcon(L"kelp_side");
}
