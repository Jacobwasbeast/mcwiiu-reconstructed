#include "game/item/ItemColors.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockColors.hpp"
#include "game/core/GrassColor.hpp"
#include "game/core/DyeColor.hpp"

template <typename T>
static arrayWithLength<T> makeColorArray(T* values, unsigned int count)
{
    std::vector<T> result(values, values + count);
    return arrayWithLength<T>(result);
}

ItemColors::ItemColors() : mItemColorIds(new IdMapper<ItemColor*>()) {}

void ItemColors::registerColor(ItemColor* color, arrayWithLength<Item*> items)
{
    for (unsigned int i = 0; i < items.length; ++i)
    {
        mItemColorIds->addMapping(color, Item::getId(items.data[i]));
    }
}

void ItemColors::registerColor(ItemColor* color, arrayWithLength<Block*> blocks)
{
    for (unsigned int i = 0; i < blocks.length; ++i)
    {
        mItemColorIds->addMapping(color, Item::getId(Item::byBlock(blocks.data[i])));
    }
}

ItemColors* ItemColors::createDefault(BlockColors* blockColors)
{
    class Armor_ItemColor : public ItemColor
    {
    public:
        int getColor(not_null_ptr<ItemInstance> item, int layer)
        {
            return layer < 1 ? item->getItem()->getColor(item, 0) : -1;
        }
    };
    class Bed_ItemColor : public ItemColor
    {
    public:
        int getColor(not_null_ptr<ItemInstance> item, int layer)
        {
            if (layer != 0) return -1;
            const DyeColor* dye = DyeColor::byBlockData(item->getAuxValue());
            return dye == NULL ? -1 : dye->getTextureDiffuseColor();
        }
    };
    class DoublePlant_ItemColor : public ItemColor
    {
    public:
        int getColor(not_null_ptr<ItemInstance> item, int)
        {
            int data = item->getAuxValue() & 7;
            return (data == 2 || data == 3) ? (int)GrassColor::get(0.5, 1.0) : -1;
        }
    };
    class FilledMap_ItemColor : public ItemColor
    {
    public:
        int getColor(not_null_ptr<ItemInstance>, int) { return -1; }
    };
    class FireworkCharge_ItemColor : public ItemColor
    {
    public:
        int getColor(not_null_ptr<ItemInstance>, int layer)
        {
            return layer == 1 ? 0x8a8a8a : -1;
        }
    };
    class Grass_ItemColor : public ItemColor {
        BlockColors* colors;
    public:
        explicit Grass_ItemColor(BlockColors* value) : colors(value) {}
        int getColor(not_null_ptr<ItemInstance>, int layer) { return colors->getColor(Blocks::grass->defaultBlockState(), NULL, NULL, layer); }
    };
    class LeatherHorseArmor_ItemColor : public ItemColor
    {
    public:
        int getColor(not_null_ptr<ItemInstance> item, int layer) { return item->getItem()->getColor(item, layer); }
    };
    class Potion_ItemColor : public ItemColor
    {
    public:
        int getColor(not_null_ptr<ItemInstance> item, int layer)
        {
            return layer < 1 ? PotionUtils::getColor(PotionUtils::getMobEffects(item)) : -1;
        }
    };
    class SpawnEgg_ItemColor : public ItemColor
    {
    public:
        int getColor(not_null_ptr<ItemInstance> item, int layer) { return item->getItem()->getColor(item, layer); }
    };
    class TallGrass_ItemColor : public ItemColor
    {
    public:
        int getColor(not_null_ptr<ItemInstance>, int) { return (int)GrassColor::get(0.5, 1.0); }
    };
    class TippedArrow_ItemColor : public ItemColor
    {
    public:
        int getColor(not_null_ptr<ItemInstance> item, int layer)
        {
            return layer == 0 ? PotionUtils::getColor(PotionUtils::getMobEffects(item)) : -1;
        }
    };

    ItemColors* colors = new ItemColors();
    Item* armor[] = { Items::leatherHelmet, Items::leatherChestplate, Items::leatherLeggings, Items::leatherBoots };
    Item* bed[] = { Items::bed };
    Item* doublePlant[] = { Item::byBlock(Blocks::double_plant) };
    Item* maps[] = { Items::map, Items::filledMap, Items::filledMap };
    Item* firework[] = { Items::fireworkCharge };
    Block* grass[] = { Blocks::grass, Blocks::tallgrass, Blocks::double_plant, Blocks::leaves, Blocks::leaves2, Blocks::vine };
    Item* horse[] = { Items::leatherHorseArmor };
    Item* potion[] = { Items::potion };
    Item* egg[] = { Items::spawnEgg };
    Item* tallGrass[] = { Item::byBlock(Blocks::tallgrass) };
    Block* tipped[] = { Blocks::tallgrass };
    colors->registerColor(new Armor_ItemColor(), makeColorArray(armor, 4));
    colors->registerColor(new Bed_ItemColor(), makeColorArray(bed, 1));
    colors->registerColor(new DoublePlant_ItemColor(), makeColorArray(doublePlant, 1));
    colors->registerColor(new FilledMap_ItemColor(), makeColorArray(maps, 3));
    colors->registerColor(new FireworkCharge_ItemColor(), makeColorArray(firework, 1));
    colors->registerColor(new Grass_ItemColor(blockColors), makeColorArray(grass, 6));
    colors->registerColor(new LeatherHorseArmor_ItemColor(), makeColorArray(horse, 1));
    colors->registerColor(new Potion_ItemColor(), makeColorArray(potion, 1));
    colors->registerColor(new SpawnEgg_ItemColor(), makeColorArray(egg, 1));
    colors->registerColor(new TallGrass_ItemColor(), makeColorArray(tallGrass, 1));
    colors->registerColor(new TippedArrow_ItemColor(), makeColorArray(tipped, 1));
    return colors;
}

int ItemColors::getColor(not_null_ptr<ItemInstance> item, int layer)
{
    ItemColor* color = mItemColorIds->byId(Item::getId(item->getItem()));
    return color == NULL ? -1 : color->getColor(item, layer);
}
