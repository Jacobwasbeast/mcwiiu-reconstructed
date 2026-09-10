#include "game/item/ClothDyeRecipes.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/FlowerBlock.hpp"

void ClothDyeRecipes::addRecipes(Recipes* recipes)
{
    int color;
    int inverseColor;

    for (color = 0, inverseColor = 15; color < 16; ++color, --inverseColor)
    {
        if (color != DyeColor::WHITE->getBlockData())
        {
            recipes->addShapelessRecipe(new ItemInstance(Blocks::wool, 1, color), L"zzg",
                new ItemInstance(Items::dye, 1, inverseColor), new ItemInstance(Item::byBlock(Blocks::wool)), 0x44);
        }
        recipes->addShapedRecipe(new ItemInstance(Blocks::stained_hardened_clay, 8, inverseColor), L"sssczczg",
            L"###", L"#X#", L"###",
            0x23, new ItemInstance(Blocks::hardened_clay), 0x58, new ItemInstance(Items::dye, 1, color), 0x44);
        recipes->addShapelessRecipe(new ItemInstance(Blocks::concrete_powder, 8, inverseColor), L"zzzzzzzzzg",
            new ItemInstance(Items::dye, 1, color), new ItemInstance(Blocks::gravel), new ItemInstance(Blocks::gravel),
            new ItemInstance(Blocks::gravel), new ItemInstance(Blocks::gravel), new ItemInstance(Blocks::sand),
            new ItemInstance(Blocks::sand), new ItemInstance(Blocks::sand), new ItemInstance(Blocks::sand), 0x44);
        recipes->addShapedRecipe(new ItemInstance(Blocks::stained_glass, 8, inverseColor), L"sssczczg",
            L"###", L"#X#", L"###",
            0x23, new ItemInstance(Blocks::glass), 0x58, new ItemInstance(Items::dye, 1, color), 0x44);
        recipes->addShapedRecipe(new ItemInstance(Blocks::stained_glass_pane, 16, inverseColor), L"ssczg", L"#X#", L"###",
            0x23, new ItemInstance(Blocks::stained_glass, 1, inverseColor), 0x44);
    }

    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::YELLOW->getItemData()), L"zg",
        new ItemInstance(Blocks::yellow_flower, 1, FlowerBlock::Flower::FLOWERS[0]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()), L"zg",
        new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::FLOWERS[1]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 3, DyeColor::WHITE->getItemData()), L"ig", Items::bone, 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 9, DyeColor::WHITE->getItemData()), L"tg", Blocks::bone_block, 0x44);

    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::PINK->getItemData()), L"zzg",
        new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::ORANGE->getItemData()), L"zzg",
        new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::YELLOW->getItemData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::LIME->getItemData()), L"zzg",
        new ItemInstance(Items::dye, 1, DyeColor::GREEN->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::GRAY->getItemData()), L"zzg",
        new ItemInstance(Items::dye, 1, DyeColor::BLACK->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::SILVER->getItemData()), L"zzg",
        new ItemInstance(Items::dye, 1, DyeColor::GRAY->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 3, DyeColor::SILVER->getItemData()), L"zzzg",
        new ItemInstance(Items::dye, 1, DyeColor::BLACK->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::LIGHT_BLUE->getItemData()), L"zzg",
        new ItemInstance(Items::dye, 1, DyeColor::BLUE->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::CYAN->getItemData()), L"zzg",
        new ItemInstance(Items::dye, 1, DyeColor::BLUE->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::GREEN->getItemData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::PURPLE->getItemData()), L"zzg",
        new ItemInstance(Items::dye, 1, DyeColor::BLUE->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::MAGENTA->getItemData()), L"zzg",
        new ItemInstance(Items::dye, 1, DyeColor::PURPLE->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::PINK->getItemData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 3, DyeColor::MAGENTA->getItemData()), L"zzzg",
        new ItemInstance(Items::dye, 1, DyeColor::BLUE->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::PINK->getItemData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 4, DyeColor::MAGENTA->getItemData()), L"zzzzg",
        new ItemInstance(Items::dye, 1, DyeColor::BLUE->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()),
        new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), L"D");

    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::LIGHT_BLUE->getItemData()), L"zg", new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::FLOWERS[2]->getData()), L"D");
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::MAGENTA->getItemData()), L"zg", new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::FLOWERS[3]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::SILVER->getItemData()), L"zg", new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::FLOWERS[4]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()), L"zg", new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::FLOWERS[5]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::ORANGE->getItemData()), L"zg", new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::FLOWERS[6]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::SILVER->getItemData()), L"zg", new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::FLOWERS[7]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::PINK->getItemData()), L"zg", new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::FLOWERS[8]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::SILVER->getItemData()), L"zg", new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::FLOWERS[9]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::YELLOW->getItemData()), L"zg", new ItemInstance(Blocks::double_plant, 1, DoublePlantBlock::VARIANTS[0]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::MAGENTA->getItemData()), L"zg", new ItemInstance(Blocks::double_plant, 1, DoublePlantBlock::VARIANTS[1]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::RED->getItemData()), L"zg", new ItemInstance(Blocks::double_plant, 1, DoublePlantBlock::VARIANTS[4]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::PINK->getItemData()), L"zg", new ItemInstance(Blocks::double_plant, 1, DoublePlantBlock::VARIANTS[5]->getData()), 0x44);
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()), L"ig", Items::beetroot, 0x44);
    for (color = 0; color < 16; ++color)
    {
        recipes->addShapedRecipe(new ItemInstance(Blocks::carpet, 3, color), L"sczg", L"##", 0x23,
            new ItemInstance(Blocks::wool, 1, color), 0x44);
    }
    for (color = 0; color < 16; ++color)
    {
        recipes->addShapelessRecipe(new ItemInstance(Items::bed, 1, color), L"zzgv",
            new ItemInstance(Items::dye, 1, 15 - color), new ItemInstance(Items::bed, 1, -1), 0x58, 0x56);
    }
}
