#include "game/world/generation/StructureRecipies.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/NewStoneSlabBlock.hpp"
#include "game/world/block/QuartzBlock.hpp"
#include "game/world/block/RedSandStoneBlock.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/StoneSlabBlock.hpp"

void StructureRecipies::addRecipes(Recipes* recipes)
{
    recipes->addShapedRecipe(new ItemInstance(Blocks::sandstone), L"ssczg", L"##", L"##", L'#', new ItemInstance(Blocks::sand, 1, SandBlock::Variant::byData(0)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::red_sandstone), L"ssczg", L"##", L"##", L'#', new ItemInstance(Blocks::sand, 1, SandBlock::Variant::byData(1)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::sandstone, 4, SandStoneBlock::Variant::byData(2)->getData()), L"ssczg", L"##", L"##", L'#', new ItemInstance(Blocks::sandstone, 1, SandStoneBlock::Variant::byData(0)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::red_sandstone, 4, RedSandStoneBlock::Variant::byData(2)->getData()), L"ssczg", L"##", L"##", L'#', new ItemInstance(Blocks::red_sandstone, 1, RedSandStoneBlock::Variant::byData(0)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::sandstone, 1, SandStoneBlock::Variant::byData(1)->getData()), L"ssczg", L"#", L"#", L'#', new ItemInstance(Blocks::stone_slab, 1, StoneSlabBlock::Variant::byData(1)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::red_sandstone, 1, RedSandStoneBlock::Variant::byData(1)->getData()), L"ssczg", L"#", L"#", L'#', new ItemInstance(Blocks::stone_slab2, 1, NewStoneSlabBlock::Variant::byData(0)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::quartz_block, 1, QuartzBlock::Variant::byData(1)->getData()), L"ssczg", L"#", L"#", L'#', new ItemInstance(Blocks::stone_slab, 1, StoneSlabBlock::Variant::byData(7)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::quartz_block, 2, QuartzBlock::Variant::byData(2)->getData()), L"ssczg", L"#", L"#", L'#', new ItemInstance(Blocks::quartz_block, 1, QuartzBlock::Variant::byData(0)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::crafting_table), L"ssctg", L"##", L"##", L'#', Blocks::planks, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::furnace), L"sssctg", L"###", L"# #", L"###", L'#', Blocks::cobblestone, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::chest), L"sssctg", L"###", L"# #", L"###", L'#', Blocks::planks, 0x53);
    recipes->addShapelessRecipe(new ItemInstance(Blocks::trapped_chest), L"sttg", Blocks::chest, Blocks::tripwire_hook, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::ender_chest), L"sssctcig", L"###", L"#E#", L"###", L'#', Blocks::obsidian, L'E', Items::enderEye, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stonebrick, 4), L"ssczg", L"##", L"##", L'#', new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::byData(0)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stonebrick, 1, 3), L"ssczg", L"#", L"#", L'#', new ItemInstance(Blocks::stone_slab, 1, StoneSlabBlock::Variant::byData(5)->getData()), 0x53);
    recipes->addShapelessRecipe(new ItemInstance(Blocks::stonebrick, 1, 1), L"ztg", new ItemInstance(Blocks::stonebrick, 1, 0), Blocks::vine, 0x53);
    recipes->addShapelessRecipe(new ItemInstance(Blocks::mossy_cobblestone, 1), L"ttg", Blocks::cobblestone, Blocks::vine, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::glass_pane, 16), L"ssctg", L"###", L"###", L'#', Blocks::glass, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::nether_brick, 1), L"sscig", L"NN", L"NN", L'N', Items::netherbrick, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::red_nether_brick, 1), L"sscicig", L"NW", L"WN", L'N', Items::netherbrick, L'W', Items::netherWart, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::redstone_lamp, 1), L"ssscictg", L" R ", L"RGR", L" R ", L'R', Items::redstone, L'G', Blocks::glowstone, 0x4d);
    recipes->addShapedRecipe(new ItemInstance(Blocks::beacon, 1), L"sssctcictg", L"GGG", L"GSG", L"OOO", L'G', Blocks::glass, L'S', Items::netherStar, L'O', Blocks::obsidian, 0x4d);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stone, 2, StoneBlock::Variant::byData(3)->getData()), L"ssctcig", L"CQ", L"QC", L'C', Blocks::cobblestone, L'Q', Items::quartz, 0x53);
    recipes->addShapelessRecipe(new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::byData(1)->getData()), L"zig", new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::byData(3)->getData()), Items::quartz, 0x53);
    recipes->addShapelessRecipe(new ItemInstance(Blocks::stone, 2, StoneBlock::Variant::byData(5)->getData()), L"ztg", new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::byData(3)->getData()), Blocks::cobblestone, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::dirt, 4, DirtBlock::Variant::byData(1)->getData()), L"ssczctg", L"DG", L"GD", L'D', new ItemInstance(Blocks::dirt, 1, DirtBlock::Variant::byData(0)->getData()), L'G', Blocks::gravel, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stone, 4, StoneBlock::Variant::byData(4)->getData()), L"ssczg", L"SS", L"SS", L'S', new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::byData(3)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stone, 4, StoneBlock::Variant::byData(2)->getData()), L"ssczg", L"SS", L"SS", L'S', new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::byData(1)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stone, 4, StoneBlock::Variant::byData(6)->getData()), L"ssczg", L"SS", L"SS", L'S', new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::byData(5)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::prismarine, 1, 0), L"sscig", L"SS", L"SS", L'S', Items::prismarineShard, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::prismarine, 1, 1), L"ssscig", L"SSS", L"SSS", L"SSS", L'S', Items::prismarineShard, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::prismarine, 1, 2), L"sssciczg", L"SSS", L"SIS", L"SSS", L'S', Items::prismarineShard, L'I', new ItemInstance(Items::dye, 1, DyeColor::BLACK->getItemData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::purpur_block, 4, 0), L"sscig", L"FF", L"FF", L'F', Items::chorusFruitPopped, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::purpur_pillar, 1, 0), L"ssctg", L"#", L"#", L'#', Blocks::purpur_slab, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::end_bricks, 4, 0), L"ssctg", L"##", L"##", L'#', Blocks::end_stone, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::magma, 1, 0), L"sscig", L"##", L"##", L'#', Items::magmaCream, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::nether_wart_block, 1, 0), L"ssscig", L"###", L"###", L"###", L'#', Items::netherWart, 0x44);
}
