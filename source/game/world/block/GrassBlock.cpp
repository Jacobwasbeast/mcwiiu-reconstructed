#include "game/world/block/GrassBlock.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
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
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<bool>* GrassBlock::SNOWY = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"snowy"));

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
    const int steps = distance / 16;
    for (int i = 0; i < steps; ++i)
    {
        const int dx = random->nextInt(3) - 1;
        const int dyDirection = random->nextInt(3) - 1;
        const int dy = dyDirection * random->nextInt(3) / 2;
        const int dz = random->nextInt(3) - 1;
        pos = pos.offset(dx, dy, dz);
        if (level->getBlockState(pos.below())->getBlock() != Blocks::grass
            || level->getBlockState(pos)->isSolidBlockingCubeAndNotSignalSource())
            return false;
    }
    return steps > 0;
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
            level->setBlock(candidate, generatedState, Block::UPDATE_ALL, false);
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

    int brightness = level->getRawBrightness(pos.above());
    if (brightness < 4)
    {
        int lightBlock = level->getBlockState(pos.above())->getLightBlock();
        int extraLightBlock = level->getExtraBlockState(pos.above())->getLightBlock();
        if (lightBlock < extraLightBlock)
            lightBlock = extraLightBlock;
        if (lightBlock > 2)
            level->setBlock(pos, Blocks::dirt->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        return;
    }
    if (brightness <= 8)
        return;

    for (int i = 0; i < 4; ++i)
    {
        int zOffset = random->nextInt(3) - 1;
        int yOffset = random->nextInt(2);
        int xOffset = random->nextInt(3) - 1;
        BlockPos target(pos.x + xOffset, pos.y + yOffset, pos.z + zOffset);
        Block* block = level->getBlock(target);
        if (target.y < 256 && !level->hasChunkAt(target))
            return;
        if (block != Blocks::dirt)
            continue;
        if (Blocks::dirt->getBlockState(level->getBlockData(target.x, target.y, target.z))->getValue(DirtBlock::VARIANT)
            != DirtBlock::Variant::DIRT)
        {
            continue;
        }

        const BlockState* aboveState = level->getBlockState(target.above());
        const BlockState* aboveExtraState = level->getExtraBlockState(target.above());
        Material* aboveMaterial = aboveState->getBlock()->mMaterial;
        Material* aboveExtraMaterial = aboveExtraState->getBlock()->mMaterial;
        int lightBlock = aboveState->getLightBlock();
        int extraLightBlock = aboveExtraState->getLightBlock();
        if (lightBlock < extraLightBlock)
            lightBlock = extraLightBlock;
        if (level->getRawBrightness(target.above()) >= 4 && !aboveState->isSolid() && !aboveMaterial->isLiquid()
            && !aboveExtraMaterial->isLiquid() && lightBlock < 3)
        {
            level->setBlock(target, Blocks::grass->defaultBlockState(), Block::UPDATE_CLIENTS, false);
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
    switch (Item::getId(tooltipData.m_pItemInstance->getItem()))
    {
    case 256:
    case 269:
    case 273:
    case 277:
    case 284:
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success && tooltipData.m_bCanPlace)
            return IDS_TOOLTIPS_DIG_PATH;
        break;
    case 290:
    case 291:
    case 292:
    case 293:
    case 294:
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success && tooltipData.m_bCanPlace)
            return IDS_TOOLTIPS_TILL;
        break;
    case 351:
        if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
            && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
        {
            return IDS_TOOLTIPS_GROW;
        }
        break;
    }
    return Block::GetInteractTooltip(tooltipData);
}

bool GrassBlock::waterSpreadCausesSpawn()
{
    return true;
}

BlockStateDefinition* GrassBlock::createBlockStateDefinition()
{
    static const Property* properties[1] = { SNOWY };
    return new BlockStateDefinition(this, properties, 1);
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
        return Blocks::dirt->getTexture(direction);
    return mIcon;
}

TextureAtlasSprite* GrassBlock::getTexture(LevelSource* levelSource, const BlockPos& pos,
                                            const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP)
        return mTopIcon;
    if (direction == Direction::DOWN)
        return Blocks::dirt->getTexture(direction);
    Material* aboveMaterial = levelSource->getBlockState(pos.above())->getBlock()->mMaterial;
    if (aboveMaterial != Material::TOP_SNOW && aboveMaterial != Material::SNOW)
        return mIcon;
    return mSnowSideIcon;
}

void GrassBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"grass_side");
    mTopIcon = iconRegister->registerIcon(L"grass_top");
    mSnowSideIcon = iconRegister->registerIcon(L"snow_side");
    mSideOverlayIcon = iconRegister->registerIcon(L"grass_side_overlay");
}

TextureAtlasSprite* GrassBlock::getSideTextureOverlay()
{
    return static_cast<GrassBlock*>(Blocks::grass)->mSideOverlayIcon;
}
