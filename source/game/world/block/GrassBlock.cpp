#include "game/world/block/GrassBlock.hpp"

#include <string>

#include "game/core/IconRegister.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Item.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

GrassBlock::GrassBlock()
    : Block(Material::GRASS)
{
    DerivedInit();
    setTicking(true);
}

GrassBlock::~GrassBlock()
{
}

bool GrassBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool)
{
    return true;
}

bool GrassBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

bool GrassBlock::randomWalk(Level* level, Random* random, BlockPos& pos, int distance)
{
    int walks = (distance + 15) / 16;
    for (int i = 0; i < walks / 2; ++i)
    {
        pos = pos.offset(random->nextInt(3) - 1,
                         ((random->nextInt(3) - 1) * random->nextInt(3)) / 2,
                         random->nextInt(3) - 1);
        const BlockState* belowState = level->getBlockState(pos.below());
        if (belowState->getBlock() != this || level->getExtraBlockState(pos)->getBlock() != NULL)
            return false;

        pos = pos.offset(random->nextInt(3) - 1,
                         ((random->nextInt(3) - 1) * random->nextInt(3)) / 2,
                         random->nextInt(3) - 1);
        belowState = level->getBlockState(pos.below());
        if (belowState->getBlock() != this || level->getExtraBlockState(pos)->getBlock() != NULL)
            return false;
    }
    if ((walks & 1) != 0)
    {
        pos = pos.offset(random->nextInt(3) - 1,
                         ((random->nextInt(3) - 1) * random->nextInt(3)) / 2,
                         random->nextInt(3) - 1);
        const BlockState* belowState = level->getBlockState(pos.below());
        if (belowState->getBlock() == this && level->getExtraBlockState(pos)->getBlock() == NULL)
            return true;
        return false;
    }
    return walks == 0;
}

void GrassBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos,
                                 const BlockState*)
{
    BlockPos target = pos.above();
    int attempts = 48;
    int distance = 16;
    while (attempts-- != 0)
    {
        BlockPos candidate = target;
        if (!randomWalk(level, random, candidate, distance))
        {
            ++distance;
            continue;
        }
        const BlockState* state = level->getBlockState(candidate);
        if (state->getBlock() != this)
            continue;

        Block* block;
        const BlockState* generatedState;
        if (random->nextInt(8) != 0)
        {
            block = Blocks::tallgrass;
            generatedState = block->defaultBlockState();
        }
        else
        {
            FlowerBlock::Flower* flower = level->getBiome(candidate)->getRandomFlower(*random, candidate);
            block = flower->getCategory()->getBlock();

            TypedBoxed<FlowerBlock::Flower*> boxedFlower(flower);
            generatedState = block->defaultBlockState()->setBoxedValue(
                static_cast<FlowerBlock*>(block)->getFlowerProperty(), &boxedFlower);
        }

        if (block->canSurvive(level, candidate, generatedState))
        {
            level->setBlock(candidate, generatedState, 3, false);
        }
    }
}

int GrassBlock::getColor() const
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Grass_Common);
}

int GrassBlock::getColor(const BlockState* state)
{
    return getColor();
}

int GrassBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int)
{
    return getColor(levelSource, pos, levelSource->getBlockState(pos));
}

int GrassBlock::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState*)
{
    return BiomeColors::getAverageGrassColor(levelSource, pos);
}

void GrassBlock::tick(Level* level, const BlockPos& pos, const BlockState*, Random* random)
{
    if (level->mIsClientSide)
        return;

    BlockPos above = pos.above();
    int brightness = level->getRawBrightness(above);
    if (brightness < 4)
    {
        const BlockState* state = level->getBlockState(above);
        const BlockState* extraState = level->getExtraBlockState(above);
        int lightBlock = state->getLightBlock();
        if (extraState->getLightBlock() > lightBlock)
            lightBlock = extraState->getLightBlock();
        if (lightBlock > 2)
        {
            level->setBlock(pos, defaultBlockState(), 2, false);
            return;
        }
    }
    else if (brightness > 8)
    {
        for (int i = 0; i < 4; ++i)
        {
            BlockPos target = pos.offset(random->nextInt(3) - 1,
                                         random->nextInt(2),
                                         random->nextInt(3) - 1);
            const BlockState* state = level->getBlockState(target);
            if (!level->hasChunkAt(target))
                return;
            if (state->getBlock() != this)
                continue;
            level->setBlock(target, defaultBlockState(), 2, false);
        }
    }
}

Item* GrassBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::dirt);
}

int GrassBlock::getRenderLayer()
{
    return 0;
}

int GrassBlock::convertBlockStateToLegacyData(const BlockState*)
{
    return 0;
}

unsigned int GrassBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    return Block::GetInteractTooltip(tooltipData);
}

bool GrassBlock::waterSpreadCausesSpawn()
{
    return true;
}

BlockStateDefinition* GrassBlock::createBlockStateDefinition()
{
    return new BlockStateDefinition(this);
}

bool GrassBlock::shouldBlockTick(Level* level, const BlockPos& pos, const BlockState*)
{
    BlockPos above = pos.above();
    int lightBlock = level->getBlockState(above)->getLightBlock();
    int extraLightBlock = level->getExtraBlockState(above)->getLightBlock();
    if (extraLightBlock > lightBlock)
        lightBlock = extraLightBlock;
    if (lightBlock > 2 && level->getRawBrightness(above) < 4)
        return true;
    return level->getRawBrightness(above) > 8;
}

TextureAtlasSprite* GrassBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::UP)
        return mTopIcon;
    if (direction == Direction::DOWN)
        return Blocks::dirt->getTexture(direction, NULL);
    return mSideIcon;
}

TextureAtlasSprite* GrassBlock::getTexture(LevelSource* levelSource, const BlockPos& pos,
                                            const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP)
        return mTopIcon;
    if (direction == Direction::DOWN)
        return mSnowSideIcon;
    const BlockState* above = levelSource->getBlockState(pos.above());
    if (above->getBlock() != Blocks::snow && above->getBlock() != Blocks::snow_layer)
        return mSideIcon;
    return mSnowSideIcon;
}

void GrassBlock::registerIcons(IconRegister* iconRegister)
{
    mSideIcon = iconRegister->registerIcon(L"grass_side");
    mTopIcon = iconRegister->registerIcon(L"grass_top");
    mSnowSideIcon = iconRegister->registerIcon(L"snow_side");
    mSideOverlayIcon = iconRegister->registerIcon(L"grass_side_overlay");
}

TextureAtlasSprite* GrassBlock::getSideTextureOverlay()
{
    return Blocks::grass == NULL ? NULL : static_cast<GrassBlock*>(Blocks::grass)->mSideOverlayIcon;
}
