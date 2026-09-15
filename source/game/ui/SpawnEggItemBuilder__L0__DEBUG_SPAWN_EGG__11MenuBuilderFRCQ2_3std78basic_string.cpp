#include "game/ui/SpawnEggItemBuilder__L0__DEBUG_SPAWN_EGG__11MenuBuilderFRCQ2_3std78basic_string.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"

SpawnEggItemBuilder__L0__DEBUG_SPAWN_EGG__11MenuBuilderFRCQ2_3std78basic_string::SpawnEggItemBuilder__L0__DEBUG_SPAWN_EGG__11MenuBuilderFRCQ2_3std78basic_string(
    const std::wstring& entityTypeValue, const std::wstring& debugTagValue)
    : UIScene_CreativeMenu::ItemBuilder(ItemInstance(Items::spawnEgg)),
      entityType(entityTypeValue), debugTag(debugTagValue)
{
}

not_null_ptr<ItemInstance>
SpawnEggItemBuilder__L0__DEBUG_SPAWN_EGG__11MenuBuilderFRCQ2_3std78basic_string::build() const
{
    not_null_ptr<ItemInstance> result = item.copy();
    SpawnEggItem::addSpawnEntity(result, entityType);
    if (!debugTag.empty())
    {
        result->getTag()->putString(L"DebugTag", debugTag);
    }
    return result;
}
