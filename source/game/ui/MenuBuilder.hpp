#ifndef GAME_UI_MENUBUILDER_HPP
#define GAME_UI_MENUBUILDER_HPP

#include <vector>

#include "game/item/FishFoodItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"

class BannerPattern;
class Block;
class BlockState;
class DyeColor;
class Item;
class Potion;

template <typename T>
class AbstractProperty;

class UIScene_CreativeMenu
{
public:
    class ItemBuilder
    {
    public:
        ItemBuilder(const ItemInstance& item);
        virtual ~ItemBuilder();

        ItemInstance item;
    };
};

class MenuBuilder
{
public:
    template <typename T>
    void ITEM_AUX(Block* block, const AbstractProperty<T>* property, T value);

    void ITEM_BLOCKSTATE(const BlockState* state);
    void FIREWORK(unsigned char flight, int explosionType, int fadeType,
                  bool flicker, bool trail, int secondExplosionType);
    void ADD(const not_null_ptr<ItemInstance>& item);
    void ITEM_AUX__tm__26_PQ2_11QuartzBlock7Variant(Block* block,
                                                     const AbstractProperty<unsigned int>* property,
                                                     unsigned int value);
    void ITEM_AUX__tm__26_PQ2_11PlanksBlock7Variant(Block* block,
                                                     const AbstractProperty<unsigned int>* property,
                                                     unsigned int value);
    void ITEM_AUX__tm__23_PQ2_9DirtBlock7Variant(Block* block,
                                                  const AbstractProperty<unsigned int>* property,
                                                  unsigned int value);
    void ITEM_AUX__tm__29_PQ2_14StoneSlabBlock7Variant(Block* block,
                                                        const AbstractProperty<unsigned int>* property,
                                                        unsigned int value);
    void ITEM_AUX__tm__32_PQ2_17NewStoneSlabBlock7Variant(Block* block,
                                                            const AbstractProperty<unsigned int>* property,
                                                            unsigned int value);
    void ITEM_AUX__tm__34_PQ2_19PrismarineSlabBlock7Variant(Block* block,
                                                              const AbstractProperty<unsigned int>* property,
                                                              unsigned int value);
    void ITEM_AUX__tm__2_b(Block* block, const AbstractProperty<bool>* property, unsigned char value);
    void ITEM_AUX__tm__29_PQ2_14TallGrassBlock7Variant(Block* block,
                                                        const AbstractProperty<unsigned int>* property,
                                                        unsigned int value);
    void ITEM_AUX__tm__25_PQ2_11FlowerBlock6Flower(Block* block,
                                                   const AbstractProperty<unsigned int>* property,
                                                   unsigned int value);
    void ITEM_AUX__tm__31_PQ2_16DoublePlantBlock7Variant(Block* block,
                                                          const AbstractProperty<unsigned int>* property,
                                                          unsigned int value);
    void BANNER(const DyeColor* color, const BannerPattern* pattern);
    void ITEM_AUX__tm__32_PQ2_17HugeMushroomBlock7Variant(Block* block,
                                                           const AbstractProperty<unsigned int>* property,
                                                           unsigned int value);
    void ITEM_AUX__tm__23_Q2_12FishFoodItem4Type(Item* item, FishFoodItem::Type* type);
    void POTION(Item* item, Potion* potion);

private:
    std::vector<const UIScene_CreativeMenu::ItemBuilder*>* mItems;
};

#endif
