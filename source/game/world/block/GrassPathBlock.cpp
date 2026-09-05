#include "game/world/block/GrassPathBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

GrassPathBlock::GrassPathBlock()
    : Block(Material::DIRT), mTopIcon(NULL)
{
    DerivedInit();
    setLightBlock(0xff);
}

GrassPathBlock::~GrassPathBlock()
{
}

bool GrassPathBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    int dataValue = direction->get3DDataValue();
    if (dataValue == Direction::eDirection_Down)
    {
        return Block::shouldRenderFace(state, levelSource, pos, direction);
    }
    if (dataValue == Direction::eDirection_Up)
    {
        return true;
    }
    if (dataValue < 6)
    {
        const BlockState* neighbourState = levelSource->getBlockState(pos.relative(direction));
        Block* neighbour = neighbourState->getBlock();
        return !neighbourState->isSolidBlockingCube()
            && neighbour != Blocks::leaves
            && neighbour != Blocks::leaves2;
    }
    return Block::shouldRenderFace(state, levelSource, pos, direction);
}

void GrassPathBlock::updateBlockState(Level* level, const BlockPos& pos)
{
    const BlockState* aboveState = level->getBlockState(pos.above());
    if (aboveState->getMaterial()->blocksMotion())
    {
        level->setBlockAndUpdate(pos, Blocks::dirt->defaultBlockState());
    }
}

void GrassPathBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    updateBlockState(level, pos);
}

bool GrassPathBlock::isSilkTouchable()
{
    return true;
}

const AABB* GrassPathBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

bool GrassPathBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool GrassPathBlock::isCubeShaped(const BlockState*)
{
    return false;
}

Item* GrassPathBlock::getResource(const BlockState*, Random* random, int lootBonus)
{
    return Blocks::dirt->getResource(Blocks::dirt->defaultBlockState(), random, lootBonus);
}

not_null_ptr<ItemInstance> GrassPathBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this));
}

void GrassPathBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    Block::neighborChanged(state, level, pos, block, fromPos);
    updateBlockState(level, pos);
}

EBlockFaceShape GrassPathBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

unsigned int GrassPathBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    int itemId = Item::getId(tooltipData.m_pItemInstance->getItem());
    if (itemId > 0x121 && itemId < 0x127
        && tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success
        && tooltipData.m_bIsInside)
    {
        return 0x70889480u;
    }
    return Block::GetInteractTooltip(tooltipData);
}

bool GrassPathBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

TextureAtlasSprite* GrassPathBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP)
    {
        return mTopIcon;
    }
    if (direction == Direction::DOWN)
    {
        return Blocks::dirt->getTexture(direction, state);
    }
    return mIcon;
}

TextureAtlasSprite* GrassPathBlock::getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP)
    {
        return mTopIcon;
    }
    if (direction == Direction::DOWN)
    {
        return Blocks::dirt->getTexture(levelSource, pos, direction, state);
    }
    return mIcon;
}

void GrassPathBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"grass_path_side");
    mTopIcon = iconRegister->registerIcon(L"grass_path_top");
}
