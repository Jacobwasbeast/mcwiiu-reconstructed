#include "game/item/PickaxeItem.hpp"

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

class Blocks
{
public:
    static Block* activator_rail;
    static Block* coal_ore;
    static Block* cobblestone;
    static Block* detector_rail;
    static Block* diamond_block;
    static Block* diamond_ore;
    static Block* double_stone_slab;
    static Block* golden_rail;
    static Block* gold_block;
    static Block* gold_ore;
    static Block* ice;
    static Block* iron_block;
    static Block* iron_ore;
    static Block* lapis_block;
    static Block* lapis_ore;
    static Block* lit_redstone_ore;
    static Block* mossy_cobblestone;
    static Block* netherrack;
    static Block* packed_ice;
    static Block* rail;
    static Block* redstone_ore;
    static Block* sandstone;
    static Block* stone;
    static Block* stone_slab;
    static Block* stone_button;
    static Block* stone_pressure_plate;
    static Block* red_sandstone;
    static Block* obsidian;
    static Block* emerald_ore;
    static Block* emerald_block;
};

static arrayWithLength<Block*> BLOCKS(26, false);

void PickaxeItem::staticCtor()
{
    BLOCKS.data[0] = Blocks::activator_rail;
    BLOCKS.data[1] = Blocks::coal_ore;
    BLOCKS.data[2] = Blocks::cobblestone;
    BLOCKS.data[3] = Blocks::detector_rail;
    BLOCKS.data[4] = Blocks::diamond_block;
    BLOCKS.data[5] = Blocks::diamond_ore;
    BLOCKS.data[6] = Blocks::double_stone_slab;
    BLOCKS.data[7] = Blocks::golden_rail;
    BLOCKS.data[8] = Blocks::gold_block;
    BLOCKS.data[9] = Blocks::gold_ore;
    BLOCKS.data[10] = Blocks::ice;
    BLOCKS.data[11] = Blocks::iron_block;
    BLOCKS.data[12] = Blocks::iron_ore;
    BLOCKS.data[13] = Blocks::lapis_block;
    BLOCKS.data[14] = Blocks::lapis_ore;
    BLOCKS.data[15] = Blocks::lit_redstone_ore;
    BLOCKS.data[16] = Blocks::mossy_cobblestone;
    BLOCKS.data[17] = Blocks::netherrack;
    BLOCKS.data[18] = Blocks::packed_ice;
    BLOCKS.data[19] = Blocks::rail;
    BLOCKS.data[20] = Blocks::redstone_ore;
    BLOCKS.data[21] = Blocks::sandstone;
    BLOCKS.data[21] = Blocks::red_sandstone;
    BLOCKS.data[22] = Blocks::stone;
    BLOCKS.data[23] = Blocks::stone_slab;
    BLOCKS.data[24] = Blocks::stone_button;
    BLOCKS.data[25] = Blocks::stone_pressure_plate;
}

PickaxeItem::PickaxeItem(const Item::Tier* tier)
    : DiggerItem(2.0f, -2.8f, tier, BLOCKS)
{
}

PickaxeItem::~PickaxeItem()
{
}

bool PickaxeItem::canDestroySpecial(const BlockState* state)
{
    Block* block = state->getBlock();

    if (block == Blocks::obsidian)
    {
        return mTier->getLevel() == 3;
    }

    if (block == Blocks::diamond_block || block == Blocks::diamond_ore ||
        block == Blocks::emerald_ore || block == Blocks::emerald_block ||
        block == Blocks::gold_block || block == Blocks::gold_ore ||
        block == Blocks::redstone_ore || block == Blocks::lit_redstone_ore)
    {
        return mTier->getLevel() >= 2;
    }

    if (block == Blocks::iron_block || block == Blocks::iron_ore ||
        block == Blocks::lapis_block || block == Blocks::lapis_ore)
    {
        return mTier->getLevel() >= 1;
    }

    Material* material = state->getMaterial();
    return material == Material::STONE || material == Material::METAL ||
           material == Material::HEAVY_METAL;
}

float PickaxeItem::getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state)
{
    Material* material = state->getMaterial();

    if (material != Material::METAL && material != Material::HEAVY_METAL &&
        material != Material::STONE)
    {
        return DiggerItem::getDestroySpeed(itemInstance, state);
    }

    return mSpeed;
}
