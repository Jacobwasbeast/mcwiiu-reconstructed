#include "game/item/ItemColors.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/FireworkChargeItem.hpp"
#include "game/data/IntArrayTag.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockColors.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/core/GrassColor.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/entity/mob/EntityIO.hpp"

template <typename T>
static arrayWithLength<T> makeColorArray(T* values, unsigned int count)
{
    std::vector<T> result(values, values + count);
    return arrayWithLength<T>(result);
}

ItemColors::ItemColors()
{
    mItemColorIds = new IdMapper<ItemColor*>();
}

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
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            return layer < 1 ? item->getItem()->getColor(item, 0) : -1;
        }
    };
    class Bed_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            const DyeColor* dye = DyeColor::byBlockData(item->getAuxValue());
            if (layer == 0)
                return static_cast<int>(Minecraft::GetInstance()->getColourTable()->getColour(dye->getBedColor()));
            return -1;
        }
    };
    class DoublePlant_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int)
        {
            DoublePlantBlock::Variant* variant =
                DoublePlantBlock::Variant::byData(item->getAuxValue());
            return (variant == gDoublePlantGrass || variant == gDoublePlantFern)
                ? static_cast<int>(GrassColor::get(0.5, 1.0)) : -1;
        }
    };
    class FilledMap_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            return layer == 0 ? -1 : MapItem::getColor(item);
        }
    };
    class FireworkCharge_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            if (layer != 1)
                return -1;
            Tag* tag = FireworkChargeItem::getExplosionTagField(item, L"Colors");
            IntArrayTag* colors = dynamic_cast<IntArrayTag*>(tag);
            if (colors == NULL)
                return 0x8a8a8a;
            arrayWithLength<int> values = colors->getAsIntArray();
            if (values.length == 1)
                return values.data[0];
            if (values.length == 0)
                return 0x8a8a8a;
            unsigned int red = 0;
            unsigned int green = 0;
            unsigned int blue = 0;
            for (unsigned int i = 0; i < values.length; ++i)
            {
                unsigned int color = static_cast<unsigned int>(values.data[i]);
                red += (color >> 16) & 0xff;
                green += (color >> 8) & 0xff;
                blue += color & 0xff;
            }
            return static_cast<int>(((red / values.length) << 16)
                | ((green / values.length) << 8)
                | (blue / values.length));
        }
    };
    class Grass_ItemColor : public ItemColor {
        BlockColors* colors;
    public:
        explicit Grass_ItemColor(BlockColors* value) : colors(value) {}
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            BlockItem* blockItem = static_cast<BlockItem*>(item->getItem());
            const BlockState* state = blockItem->getBlock()->getBlockState(item->getAuxValue());
            return colors->getColor(state, NULL, NULL, layer);
        }
    };
    class LeatherHorseArmor_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer) { return item->getItem()->getColor(item, layer); }
    };
    class Potion_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            return layer < 1 ? PotionUtils::getColor(PotionUtils::getMobEffects(item)) : -1;
        }
    };
    class SpawnEgg_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            EntityIO::SpawnInfo::iterator found = EntityIO::sSpawnInfo.find(SpawnEggItem::getNamedIdFrom(item));
            if (found == EntityIO::sSpawnInfo.end())
                return -1;
            EntityIO::SpawnableMobInfo* info = found->second;
            eMinecraftColour colour = layer == 0 ? info->colour : info->secondaryColour;
            if (colour == 0)
                return -1;
            return static_cast<int>(Minecraft::GetInstance()->getColourTable()->getColour(colour));
        }
    };
    class TallGrass_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>&, int) { return (int)GrassColor::get(0.5, 1.0); }
    };
    class TippedArrow_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            return layer == 0 ? PotionUtils::getColor(PotionUtils::getMobEffects(item)) : -1;
        }
    };

    ItemColors* colors = new ItemColors();
    Item* armor[] = { Items::leatherHelmet, Items::leatherChestplate, Items::leatherLeggings, Items::leatherBoots };
    Block* doublePlant[] = { Blocks::double_plant };
    Item* firework[] = { Items::fireworkCharge };
    Item* potions[] = { Items::potion, Items::splashPotion, Items::lingeringPotion };
    Item* spawnEgg[] = { Items::spawnEgg };
    Block* foliage[] = { Blocks::grass, Blocks::tallgrass, Blocks::vine, Blocks::leaves, Blocks::leaves2, Blocks::waterlily };
    Item* tippedArrow[] = { Items::tippedArrow };
    Item* filledMap[] = { Items::filledMap };
    Item* bed[] = { Items::bed };
    Item* horseArmor[] = { Items::leatherHorseArmor };
    Block* tallGrass[] = { Blocks::tallgrass };
    colors->registerColor(new Armor_ItemColor(), makeColorArray(armor, 4));
    colors->registerColor(new DoublePlant_ItemColor(), makeColorArray(doublePlant, 1));
    colors->registerColor(new FireworkCharge_ItemColor(), makeColorArray(firework, 1));
    colors->registerColor(new Potion_ItemColor(), makeColorArray(potions, 3));
    colors->registerColor(new SpawnEgg_ItemColor(), makeColorArray(spawnEgg, 1));
    colors->registerColor(new Grass_ItemColor(blockColors), makeColorArray(foliage, 6));
    colors->registerColor(new TippedArrow_ItemColor(), makeColorArray(tippedArrow, 1));
    colors->registerColor(new FilledMap_ItemColor(), makeColorArray(filledMap, 1));
    colors->registerColor(new Bed_ItemColor(), makeColorArray(bed, 1));
    colors->registerColor(new LeatherHorseArmor_ItemColor(), makeColorArray(horseArmor, 1));
    colors->registerColor(new TallGrass_ItemColor(), makeColorArray(tallGrass, 1));
    return colors;
}

int ItemColors::getColor(not_null_ptr<ItemInstance> item, int layer)
{
    ItemColor* color = mItemColorIds->byId(Item::getId(item->getItem()));
    return color == NULL ? -1 : color->getColor(item, layer);
}
