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
    recipes->addShapedRecipe(new ItemInstance(Blocks::sandstone), L"##", L"##", '#', new ItemInstance(Blocks::sand, 1, SandBlock::Variant::byData(0)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::sandstone), L"##", L"##", '#', new ItemInstance(Blocks::sand, 1, SandBlock::Variant::byData(1)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::sandstone, 4, SandStoneBlock::Variant::byData(2)->getData()), L"##", L"##", '#', new ItemInstance(Blocks::sandstone, 1, SandStoneBlock::Variant::byData(1)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::red_sandstone, 4, RedSandStoneBlock::Variant::byData(2)->getData()), L"##", L"##", '#', new ItemInstance(Blocks::red_sandstone, 1, RedSandStoneBlock::Variant::byData(1)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::sandstone, 1, SandStoneBlock::Variant::byData(1)->getData()), L"##", L"##", '#', new ItemInstance(Blocks::stone_slab, 1, StoneSlabBlock::Variant::byData(1)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::red_sandstone, 1, RedSandStoneBlock::Variant::byData(1)->getData()), L"##", L"##", '#', new ItemInstance(Blocks::stone_slab2, 1, NewStoneSlabBlock::Variant::byData(1)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::quartz_block, 1, QuartzBlock::Variant::byData(2)->getData()), L"##", L"##", '#', new ItemInstance(Blocks::stone_slab, 1, StoneSlabBlock::Variant::byData(1)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::quartz_block, 2, QuartzBlock::Variant::byData(3)->getData()), L"##", L"##", '#', new ItemInstance(Blocks::quartz_block, 1, QuartzBlock::Variant::byData(1)->getData()), 0x53);

    recipes->addShapedRecipe(new ItemInstance(Blocks::stone_slab), L"##", L"##", '#', Blocks::stone, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stone_slab2), L"###", L"# #", L"###", '#', Blocks::sandstone, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stone_slab2), L"###", L"# #", L"###", '#', Blocks::red_sandstone, 0x53);
    recipes->addShapelessRecipe(new ItemInstance(Blocks::stone_slab), L"##", new ItemInstance(Blocks::stone_slab, 1, StoneSlabBlock::Variant::byData(0)->getData()), new ItemInstance(Blocks::stone_slab, 1, StoneSlabBlock::Variant::byData(1)->getData()), 0x53);
    recipes->addShapelessRecipe(new ItemInstance(Blocks::stone_slab, 1, 1), L"##", Blocks::stone_slab, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stone_slab2, 16), L"###", L"###", L"###", '#', Blocks::stone, 0x44);

    recipes->addShapedRecipe(new ItemInstance(Blocks::stonebrick, 1, 0), L"##", L"##", '#', new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::byData(4)->getData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stonebrick, 1, 1), L"###", L"###", L"###", '#', Blocks::stonebrick, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stonebrick, 1, 2), L"###", L"###", L"###", '#', Blocks::stonebrick, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stonebrick, 1, 3), L"###", L"###", L"###", '#', Blocks::stonebrick, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stonebrick, 1, 4), L"##", L"##", '#', Blocks::stonebrick, 0x53);

    recipes->addShapedRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, 0), L"##", L"##", '#', Blocks::clay, 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stained_glass, 1, 0), L"###", L"#X#", L"###", '#', Blocks::glass, 'X', new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), 0x53);
    recipes->addShapedRecipe(new ItemInstance(Blocks::glass_pane, 16), L"###", L"###", '#', Blocks::glass, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::iron_bars, 16), L"###", L"###", '#', Items::ironIngot, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::torch, 4), L"#", L"X", '#', Items::coal, 'X', Items::stick, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::redstone_torch), L"#", L"X", '#', Items::redstone, 'X', Items::stick, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::redstone_lamp), L"###", L"#X#", L"###", '#', Items::glowstoneDust, 'X', Blocks::redstone_wire, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::wooden_pressure_plate), L"##", '#', Blocks::planks, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::stone_pressure_plate), L"##", '#', Blocks::stone, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::heavy_weighted_pressure_plate), L"##", '#', Blocks::iron_block, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::light_weighted_pressure_plate), L"##", '#', Blocks::gold_block, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::ladder, 3), L"# #", L"###", L"# #", '#', Items::stick, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::fence, 3), L"#X#", L"#X#", '#', Blocks::planks, 'X', Items::stick, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::fence_gate), L"#X#", L"#X#", '#', Blocks::planks, 'X', Items::stick, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::trapdoor, 2), L"###", L"###", '#', Blocks::planks, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::chest), L"###", L"# #", L"###", '#', Blocks::planks, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::trapped_chest), L"###", L"#X#", L"###", '#', Blocks::planks, 'X', Items::string, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::crafting_table), L"##", L"##", '#', Blocks::planks, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::bookshelf), L"###", L"XXX", L"###", '#', Blocks::planks, 'X', Items::book, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::jukebox), L"###", L"#X#", L"###", '#', Blocks::planks, 'X', Items::diamond, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::beacon), L"###", L"#X#", L"###", '#', Blocks::glass, 'X', Items::netherStar, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::bone_block), L"###", L"###", L"###", '#', Items::bone, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::quartz_block), L"###", L"###", L"###", '#', Items::quartz, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Blocks::end_bricks), L"##", L"##", '#', Blocks::end_stone, 0x44);
}
