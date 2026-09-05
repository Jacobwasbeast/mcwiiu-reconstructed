#include "game/ui/MenuBuilder.hpp"

#include <string>
#include <vector>

#include "game/core/BannerPattern.hpp"
#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TypedBoxed.hpp"

class BannerItem
{
public:
    static ItemInstance* makeBanner(const DyeColor* color, ListTag* patterns);
};

class BannerBlockEntity
{
public:
    static void addTags(const not_null_ptr<ItemInstance>& item, int color, ListTag* patterns);
    static void fjAddPatternToItem(const not_null_ptr<ItemInstance>& item,
                                   const BannerPattern* pattern, const DyeColor* color);
};

UIScene_CreativeMenu::ItemBuilder::ItemBuilder(const ItemInstance& value)
    : item(value)
{
}

UIScene_CreativeMenu::ItemBuilder::~ItemBuilder()
{
}

static void addItem(std::vector<const UIScene_CreativeMenu::ItemBuilder*>* items,
                    const ItemInstance& item)
{
    items->push_back(new UIScene_CreativeMenu::ItemBuilder(item));
}

void MenuBuilder::ITEM_BLOCKSTATE(const BlockState* state)
{
    Block* block = state->getBlock();
    ItemInstance item(block, 1, block->convertBlockStateToLegacyData(state));
    addItem(mItems, item);
}

void MenuBuilder::ADD(const not_null_ptr<ItemInstance>& item)
{
    addItem(mItems, *item);
}

template <typename T>
void MenuBuilder::ITEM_AUX(Block* block, const AbstractProperty<T>* property, T value)
{
    TypedBoxed<T> boxed(value);
    const BlockState* state = block->defaultBlockState()->setBoxedValue(property, &boxed);
    ItemInstance item(block, 1, block->convertBlockStateToLegacyData(state));
    addItem(mItems, item);
}

void MenuBuilder::ITEM_AUX__tm__26_PQ2_11QuartzBlock7Variant(
    Block* block, const AbstractProperty<unsigned int>* property, unsigned int value)
{
    ITEM_AUX<unsigned int>(block, property, value);
}

void MenuBuilder::ITEM_AUX__tm__26_PQ2_11PlanksBlock7Variant(
    Block* block, const AbstractProperty<unsigned int>* property, unsigned int value)
{
    ITEM_AUX<unsigned int>(block, property, value);
}

void MenuBuilder::ITEM_AUX__tm__23_PQ2_9DirtBlock7Variant(
    Block* block, const AbstractProperty<unsigned int>* property, unsigned int value)
{
    ITEM_AUX<unsigned int>(block, property, value);
}

void MenuBuilder::ITEM_AUX__tm__29_PQ2_14StoneSlabBlock7Variant(
    Block* block, const AbstractProperty<unsigned int>* property, unsigned int value)
{
    ITEM_AUX<unsigned int>(block, property, value);
}

void MenuBuilder::ITEM_AUX__tm__32_PQ2_17NewStoneSlabBlock7Variant(
    Block* block, const AbstractProperty<unsigned int>* property, unsigned int value)
{
    ITEM_AUX<unsigned int>(block, property, value);
}

void MenuBuilder::ITEM_AUX__tm__34_PQ2_19PrismarineSlabBlock7Variant(
    Block* block, const AbstractProperty<unsigned int>* property, unsigned int value)
{
    ITEM_AUX<unsigned int>(block, property, value);
}

void MenuBuilder::ITEM_AUX__tm__29_PQ2_14TallGrassBlock7Variant(
    Block* block, const AbstractProperty<unsigned int>* property, unsigned int value)
{
    ITEM_AUX<unsigned int>(block, property, value);
}

void MenuBuilder::ITEM_AUX__tm__25_PQ2_11FlowerBlock6Flower(
    Block* block, const AbstractProperty<unsigned int>* property, unsigned int value)
{
    ITEM_AUX<unsigned int>(block, property, value);
}

void MenuBuilder::ITEM_AUX__tm__31_PQ2_16DoublePlantBlock7Variant(
    Block* block, const AbstractProperty<unsigned int>* property, unsigned int value)
{
    ITEM_AUX<unsigned int>(block, property, value);
}

void MenuBuilder::ITEM_AUX__tm__32_PQ2_17HugeMushroomBlock7Variant(
    Block* block, const AbstractProperty<unsigned int>* property, unsigned int value)
{
    ITEM_AUX<unsigned int>(block, property, value);
}

void MenuBuilder::ITEM_AUX__tm__2_b(Block* block, const AbstractProperty<bool>* property,
                                    unsigned char value)
{
    ITEM_AUX<bool>(block, property, value != 0);
}

void MenuBuilder::BANNER(const DyeColor* color, const BannerPattern* pattern)
{
    ListTag* patterns = NULL;
    not_null_ptr<ItemInstance> item(BannerItem::makeBanner(color, patterns));
    BannerBlockEntity::addTags(item, color->getItemData(), patterns);
    BannerBlockEntity::fjAddPatternToItem(item, pattern, DyeColor::WHITE);
    addItem(mItems, *item);
}

void MenuBuilder::ITEM_AUX__tm__23_Q2_12FishFoodItem4Type(Item* item, FishFoodItem::Type* type)
{
    addItem(mItems, ItemInstance(item, 1, type->getData()));
}

void MenuBuilder::POTION(Item* item, Potion* potion)
{
    ItemInstance instance(item);
    class PotionItemBuilder : public UIScene_CreativeMenu::ItemBuilder
    {
    public:
        PotionItemBuilder(const ItemInstance& value, Potion* potionValue)
            : UIScene_CreativeMenu::ItemBuilder(value), potion(potionValue)
        {
        }

        Potion* potion;
    };

    mItems->push_back(new PotionItemBuilder(instance, potion));
}

void MenuBuilder::FIREWORK(unsigned char flight, int explosionType, int fadeType,
                           bool flicker, bool trail, int secondExplosionType)
{
    CompoundTag* fireworks = new CompoundTag();
    std::vector<int> explosions;
    explosions.push_back(explosionType);
    if (secondExplosionType != -1)
    {
        explosions.push_back(secondExplosionType);
    }
    if (flicker)
    {
        fireworks->putBoolean(L"Flicker", true);
    }
    if (trail)
    {
        fireworks->putBoolean(L"Trail", true);
    }
    fireworks->putIntArray(L"Explosions", arrayWithLength<int>(explosions));
    fireworks->putByte(L"Flight", flight);
    fireworks->putByte(L"Type", static_cast<unsigned char>(fadeType));

    ItemInstance item(Items::fireworks);
    item.setTag(fireworks);
    addItem(mItems, item);
}
