#ifndef GAME_UI_SPAWNEGGITEMBUILDER_L0_DEBUG_SPAWN_EGG_MENUBUILDER_HPP
#define GAME_UI_SPAWNEGGITEMBUILDER_L0_DEBUG_SPAWN_EGG_MENUBUILDER_HPP

#include <string>

#include "game/item/ItemInstance.hpp"
#include "game/ui/UIScene_CreativeMenu.hpp"
#include "game/util/ResourceLocation.hpp"

class SpawnEggItemBuilder__L0__DEBUG_SPAWN_EGG__11MenuBuilderFRCQ2_3std78basic_string
    : public UIScene_CreativeMenu::ItemBuilder
{
public:
    SpawnEggItemBuilder__L0__DEBUG_SPAWN_EGG__11MenuBuilderFRCQ2_3std78basic_string(
        const std::wstring& entityTypeValue, const std::wstring& debugTagValue);

    virtual ~SpawnEggItemBuilder__L0__DEBUG_SPAWN_EGG__11MenuBuilderFRCQ2_3std78basic_string()
    {
    }

    not_null_ptr<ItemInstance> build() const;

private:
    ResourceLocation entityType;
    std::wstring debugTag;
};

#endif
