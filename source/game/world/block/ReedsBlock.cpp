#include "game/world/block/ReedsBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
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
#include "game/world/block/TypedBoxed.hpp"
#include "game/core/DyeColor.hpp"

static AbstractProperty<int>* AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 15));

ReedsBlock::ReedsBlock()
    : Block(Material::PLANT)
{
    DerivedInit();
    TypedBoxed<int> age(0);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(AGE, &age));
    setTicking(true);
}

ReedsBlock::~ReedsBlock()
{
}

bool ReedsBlock::isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState*, bool)
{
    int bottom = pos.y;
    int lower = bottom - 4;
    while (lower < bottom - 1 && bottom - 1 > 0 && level->getBlockId(pos.x, bottom - 1, pos.z) == 83)
    {
        --lower;
        --bottom;
    }

    int upper = pos.y + 4;
    int top = pos.y;
    while (top + 1 < upper && top + 1 < level->getMaxBuildHeight() && level->getBlockId(pos.x, top + 1, pos.z) == 83)
    {
        ++upper;
        ++top;
    }

    return 2 - (top - bottom) > 0;
}

bool ReedsBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

void ReedsBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState*)
{
    int bottom = pos.y;
    int lower = bottom - 4;
    while (lower < bottom - 1 && bottom - 1 > 0
           && level->getBlockId(pos.x, bottom - 1, pos.z) == 83)
    {
        --lower;
        --bottom;
    }

    int upper = pos.y + 4;
    int top = pos.y;
    while (top + 1 < upper && top + 1 < level->getMaxBuildHeight()
           && level->getBlockId(pos.x, top + 1, pos.z) == 83)
    {
        ++upper;
        ++top;
    }

    int targetTop = top + (2 - (top - bottom));
    for (int y = top + 1; y <= targetTop; ++y)
    {
        BlockPos placement(pos.x, y, pos.z);
        if (!level->isEmptyBlock(placement))
        {
            return;
        }
        level->setBlockAndData(pos.x, y, pos.z, 83, 0, 3);
    }
}

const AABB* ReedsBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

void ReedsBlock::checkAlive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (canSurvive(level, pos, state))
    {
        return;
    }

    spawnResources(level, pos, state, 0);
    level->removeBlock(pos);
}

void ReedsBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (level->getBlockState(pos.below())->getBlock() != this)
    {
        checkAlive(level, pos, state);
    }

    if (!level->isEmptyBlock(pos.above()))
    {
        return;
    }

    int height = 1;
    if (level->getBlockState(pos.below())->getBlock() == this)
    {
        while (level->getBlockState(pos.below(height))->getBlock() == this)
        {
            ++height;
        }
        if (height > 2)
        {
            return;
        }
    }

    int age = convertBlockStateToLegacyData(state);
    if (age == 15)
    {
        level->setBlockAndData(pos.x, pos.y + 1, pos.z, 83, 0, 4);
        TypedBoxed<int> reset(0);
        level->setBlock(pos, state->setBoxedValue(AGE, &reset), 4, false);
    }
    else
    {
        TypedBoxed<int> next(age + 1);
        level->setBlock(pos, state->setBoxedValue(AGE, &next), 4, false);
    }
}

bool ReedsBlock::mayPlace(Level* level, const BlockPos& pos)
{
    const Block* below = level->getBlockState(pos.below())->getBlock();
    if (below == this)
    {
        return true;
    }

    const Block* current = level->getBlockState(pos)->getBlock();
    if (current != Blocks::AIR && current != this)
    {
        return false;
    }

    if (below != Blocks::sand && below != Blocks::dirt && below != Blocks::grass)
    {
        return false;
    }

    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        BlockPos adjacent = pos.below().relative(directions.data[i]);
        const BlockState* adjacentState = level->getBlockState(adjacent);
        const BlockState* liquidState = level->getLiquidBlockState(adjacent);
        if (adjacentState->getMaterial() == Material::WATER
            || liquidState->getMaterial() == Material::WATER
            || adjacentState->getBlock() == Blocks::ice)
        {
            return true;
        }
    }
    return false;
}

void ReedsBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                 Block*, const BlockPos&)
{
    if (!canSurvive(level, pos, state))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
}

bool ReedsBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    return mayPlace(level, pos);
}

const AABB* ReedsBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

Item* ReedsBlock::getResource(const BlockState*, Random*, int)
{
    return Items::reeds;
}

bool ReedsBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool ReedsBlock::isCubeShaped(const BlockState*)
{
    return false;
}

not_null_ptr<ItemInstance> ReedsBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::reeds));
}

int ReedsBlock::getColor(LevelSource*, const BlockPos&, int)
{
    return 0xFFFFFF;
}

int ReedsBlock::getRenderLayer()
{
    return 0;
}

const BlockState* ReedsBlock::getBlockState(int data)
{
    TypedBoxed<int> age(data);
    return defaultBlockState()->setBoxedValue(AGE, &age);
}

int ReedsBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(AGE);
}

bool ReedsBlock::waterSpreadCausesSpawn()
{
    return true;
}

bool ReedsBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

BlockStateDefinition* ReedsBlock::createBlockStateDefinition()
{
    const Property* properties[] = { AGE };
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape ReedsBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

bool ReedsBlock::shouldBlockTick(Level* level, const BlockPos& pos, const BlockState*)
{
    return level->isEmptyBlock(pos.above());
}

EBlockRenderShape ReedsBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_ENTITY;
}

unsigned int ReedsBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == 351
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail)
    {
        if (DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
        {
            return 0x9F5FDC40u;
        }
    }
    return Block::GetInteractTooltip(tooltipData);
}
