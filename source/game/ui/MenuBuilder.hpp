#ifndef GAME_UI_MENUBUILDER_HPP
#define GAME_UI_MENUBUILDER_HPP

#include <string>
#include <vector>

#include "game/item/ItemInstance.hpp"
#include "game/item/ItemInstanceClonerBuilder.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/UIScene_CreativeMenu.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TypedBoxed.hpp"

class BannerPattern;
class DyeColor;
class Item;
class Potion;

class MenuBuilder
{
public:
    template <typename T>
    void ITEM_AUX(Block* block, const AbstractProperty<T>* property, T value);

    template <typename T>
    void ITEM_AUX(Item* item, T* type);

    void ITEM_BLOCKSTATE(const BlockState* state);
    void ADD(const not_null_ptr<ItemInstance>& item);
    void BANNER(const DyeColor* color, const BannerPattern* pattern);
    void FIREWORK(unsigned char type, int colour, int flight, bool flicker, bool trail, int fadeColour);
    void POTION(Item* item, Potion* potion);
    void DEBUG_SPAWN_EGG(const std::wstring& entityId, const std::wstring& debugTag);

    std::vector<const UIScene_CreativeMenu::ItemBuilder*>* mItems;
};

template <typename T>
void MenuBuilder::ITEM_AUX(Block* block, const AbstractProperty<T>* property, T value)
{
    TypedBoxed<T> boxed(value);
    ITEM_BLOCKSTATE(block->defaultBlockState()->setBoxedValue(property, &boxed));
}

template <typename T>
void MenuBuilder::ITEM_AUX(Item* item, T* type)
{
    ItemInstance instance(item, 1, type->getData());
    mItems->push_back(new ItemInstanceClonerBuilder(instance));
}

#endif
