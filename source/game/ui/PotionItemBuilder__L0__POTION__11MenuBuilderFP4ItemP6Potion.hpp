#ifndef GAME_UI_POTIONITEMBUILDER_L0_POTION_MENUBUILDER_HPP
#define GAME_UI_POTIONITEMBUILDER_L0_POTION_MENUBUILDER_HPP

#include "game/item/PotionUtils.hpp"
#include "game/ui/UIScene_CreativeMenu.hpp"

class Potion;

class PotionItemBuilder__L0__POTION__11MenuBuilderFP4ItemP6Potion
    : public UIScene_CreativeMenu::ItemBuilder
{
public:
    PotionItemBuilder__L0__POTION__11MenuBuilderFP4ItemP6Potion(
        const ItemInstance& itemValue, Potion* potionValue)
        : UIScene_CreativeMenu::ItemBuilder(itemValue), potion(potionValue)
    {
    }

    virtual ~PotionItemBuilder__L0__POTION__11MenuBuilderFP4ItemP6Potion()
    {
    }

    not_null_ptr<ItemInstance> build() const;

private:
    Potion* potion;
};

#endif
