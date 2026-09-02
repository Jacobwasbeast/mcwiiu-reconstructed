#include "game/world/block/BlockColors.hpp"

#include <cstddef>

#include <boost/shared_ptr.hpp>

#include "game/core/ColourTable.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/FoliageColor.hpp"
#include "game/core/GrassColor.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/item/Minecraft.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/RedStoneWireBlock.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/StemBlock.hpp"
#include "game/world/block/TallGrassBlock.hpp"

BlockColors::BlockColors()
{
    mBlockColorIds = new IdMapper<BlockColor*>();
}

void BlockColors::registerColor(BlockColor* color, arrayWithLength<Block*> blocks)
{
    for (unsigned int i = 0; i < blocks.length; i++)
    {
        mBlockColorIds->addMapping(color, Block::getId(blocks.data[i]));
    }
}

BlockColors* BlockColors::createDefault()
{
    BlockColors* blockColors = new BlockColors();
    arrayWithLength<Block*> blocks;

    class DoublePlant_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            DoublePlantBlock::Variant* variant = state->getValue(DoublePlantBlock::VARIANT);

            if (levelSource == NULL || pos == NULL
                || (variant != DoublePlantBlock::GRASS && variant != DoublePlantBlock::FERN))
            {
                return -1;
            }

            BlockPos plantPos = state->getValue(DoublePlantBlock::HALF) == DoublePlantBlock::UPPER
                                    ? pos->below()
                                    : *pos;

            return BiomeColors::getAverageGrassColor(levelSource, plantPos);
        }
    };

    Block* doublePlantBlocks[] = { Blocks::double_plant };
    blocks.data = doublePlantBlocks;
    blocks.length = 1;
    blockColors->registerColor(new DoublePlant_BlockColor(), blocks);

    class FlowerPot_BlockColor : public BlockColor
    {
    public:
        FlowerPot_BlockColor(BlockColors* blockColors)
        {
            mBlockColors = blockColors;
        }

        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            if (levelSource != NULL && pos != NULL)
            {
                boost::shared_ptr<FlowerPotBlockEntity> flowerPot =
                    boost::dynamic_pointer_cast<FlowerPotBlockEntity>(levelSource->getBlockEntity(*pos));

                if (flowerPot)
                {
                    return mBlockColors->getColor(Block::byItem(flowerPot->getPlantItem())->defaultBlockState(),
                                                  levelSource,
                                                  pos,
                                                  tintIndex);
                }
            }

            return -1;
        }

    private:
        BlockColors* mBlockColors;
    };

    Block* flowerPotBlocks[] = { Blocks::flower_pot };
    blocks.data = flowerPotBlocks;
    blocks.length = 1;
    blockColors->registerColor(new FlowerPot_BlockColor(blockColors), blocks);

    class Grass_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            if (levelSource != NULL && pos != NULL)
            {
                return BiomeColors::getAverageGrassColor(levelSource, *pos);
            }

            return GrassColor::get(0.5, 1.0);
        }
    };

    Block* grassBlocks[] = { Blocks::grass };
    blocks.data = grassBlocks;
    blocks.length = 1;
    blockColors->registerColor(new Grass_BlockColor(), blocks);

    class Leaves1_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            PlanksBlock::Variant* variant = state->getValue(OldLeafBlock::VARIANT);

            if (variant == PlanksBlock::SPRUCE)
            {
                return FoliageColor::getEvergreenColor();
            }

            if (variant == PlanksBlock::BIRCH)
            {
                return FoliageColor::getBirchColor();
            }

            if (levelSource != NULL && pos != NULL)
            {
                return BiomeColors::getAverageFoliageColor(levelSource, *pos);
            }

            return FoliageColor::getDefaultColor();
        }
    };

    Block* leavesBlocks[] = { Blocks::leaves };
    blocks.data = leavesBlocks;
    blocks.length = 1;
    blockColors->registerColor(new Leaves1_BlockColor(), blocks);

    class Leaves2_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            if (levelSource != NULL && pos != NULL)
            {
                return BiomeColors::getAverageFoliageColor(levelSource, *pos);
            }

            return FoliageColor::getDefaultColor();
        }
    };

    Block* leaves2Blocks[] = { Blocks::leaves2 };
    blocks.data = leaves2Blocks;
    blocks.length = 1;
    blockColors->registerColor(new Leaves2_BlockColor(), blocks);

    class Water_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            if (levelSource != NULL && pos != NULL)
            {
                return BiomeColors::getAverageWaterColor(levelSource, *pos);
            }

            return -1;
        }
    };

    Block* waterBlocks[] = { Blocks::WATER, Blocks::FLOWING_WATER };
    blocks.data = waterBlocks;
    blocks.length = 2;
    blockColors->registerColor(new Water_BlockColor(), blocks);

    class RedstoneWire_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            return RedStoneWireBlock::getColorForData(state->getValue(RedStoneWireBlock::POWER));
        }
    };

    Block* redstoneWireBlocks[] = { Blocks::redstone_wire };
    blocks.data = redstoneWireBlocks;
    blocks.length = 1;
    blockColors->registerColor(new RedstoneWire_BlockColor(), blocks);

    class Reeds_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            return 0xFFFFFF;
        }
    };

    Block* reedsBlocks[] = { Blocks::reeds };
    blocks.data = reedsBlocks;
    blocks.length = 1;
    blockColors->registerColor(new Reeds_BlockColor(), blocks);

    class Stem_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            int age = state->getValue(StemBlock::AGE);

            int red = age * 32;
            int green = 255 - age * 8;
            int blue = age * 4;

            return red << 16 | green << 8 | blue;
        }
    };

    Block* stemBlocks[] = { Blocks::melon_stem, Blocks::pumpkin_stem };
    blocks.data = stemBlocks;
    blocks.length = 2;
    blockColors->registerColor(new Stem_BlockColor(), blocks);

    class TallGrass_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            if (levelSource != NULL && pos != NULL)
            {
                return BiomeColors::getAverageGrassColor(levelSource, *pos);
            }

            if (state->getValue(TallGrassBlock::VARIANT) == TallGrassBlock::DEAD_BUSH)
            {
                return 0xFFFFFF;
            }

            return GrassColor::get(0.5, 1.0);
        }
    };

    Block* tallGrassBlocks[] = { Blocks::tallgrass };
    blocks.data = tallGrassBlocks;
    blocks.length = 1;
    blockColors->registerColor(new TallGrass_BlockColor(), blocks);

    class Vine_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            if (levelSource != NULL && pos != NULL)
            {
                return BiomeColors::getAverageFoliageColor(levelSource, *pos);
            }

            return FoliageColor::getDefaultColor();
        }
    };

    Block* vineBlocks[] = { Blocks::vine };
    blocks.data = vineBlocks;
    blocks.length = 1;
    blockColors->registerColor(new Vine_BlockColor(), blocks);

    class WaterLily_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Tile_WaterLily);
        }
    };

    Block* waterLilyBlocks[] = { Blocks::waterlily };
    blocks.data = waterLilyBlocks;
    blocks.length = 1;
    blockColors->registerColor(new WaterLily_BlockColor(), blocks);

    class ShulkerBox_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            ShulkerBoxBlock* shulkerBox = static_cast<ShulkerBoxBlock*>(state->getBlock());

            return Minecraft::GetInstance()->getColourTable()->getColour(shulkerBox->getDyeColor()->getShulkerColor());
        }
    };

    Block* shulkerBoxBlocks[] = { Blocks::white_shulker_box,
                                  Blocks::orange_shulker_box,
                                  Blocks::magenta_shulker_box,
                                  Blocks::light_blue_shulker_box,
                                  Blocks::yellow_shulker_box,
                                  Blocks::lime_shulker_box,
                                  Blocks::pink_shulker_box,
                                  Blocks::gray_shulker_box,
                                  Blocks::silver_shulker_box,
                                  Blocks::cyan_shulker_box,
                                  Blocks::purple_shulker_box,
                                  Blocks::blue_shulker_box,
                                  Blocks::brown_shulker_box,
                                  Blocks::green_shulker_box,
                                  Blocks::red_shulker_box,
                                  Blocks::black_shulker_box };
    blocks.data = shulkerBoxBlocks;
    blocks.length = 16;
    blockColors->registerColor(new ShulkerBox_BlockColor(), blocks);

    return blockColors;
}

int BlockColors::getColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    BlockColor* color = mBlockColorIds->byId(Block::getId(state->getBlock()));

    if (color != NULL)
    {
        return color->getColor(state, NULL, NULL, 0);
    }

    const MaterialColor* mapColor = state->getMapColor(levelSource, pos);

    if (mapColor != NULL)
    {
        return Minecraft::GetInstance()->getColourTable()->getColour(mapColor->col);
    }

    return -1;
}

int BlockColors::getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
{
    BlockColor* color = mBlockColorIds->byId(Block::getId(state->getBlock()));

    if (color == NULL)
    {
        return -1;
    }

    return color->getColor(state, levelSource, pos, tintIndex);
}
