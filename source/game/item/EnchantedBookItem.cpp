#include "game/item/EnchantedBookItem.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/item/ItemInstance.hpp"

EnchantedBookItem::EnchantedBookItem()
    : Item()
{
}

EnchantedBookItem::~EnchantedBookItem()
{
}

bool EnchantedBookItem::isFoil(not_null_ptr<ItemInstance> itemInstance)
{
    return true;
}

bool EnchantedBookItem::isEnchantable(not_null_ptr<ItemInstance> itemInstance)
{
    return false;
}

ListTag* EnchantedBookItem::getEnchantments(not_null_ptr<ItemInstance> itemInstance)
{
    CompoundTag* tag = itemInstance->getTag();
    if (tag != NULL)
    {
        return tag->getList(L"StoredEnchantments", 10);
    }
    return new ListTag(0);
}

const Rarity* EnchantedBookItem::getRarity(not_null_ptr<ItemInstance> itemInstance)
{
    ListTag* enchantments = getEnchantments(itemInstance);
    if (enchantments != NULL && enchantments->size() != 0)
    {
        return Item::getRarity(itemInstance);
    }
    return Item::getRarity(itemInstance);
}

void EnchantedBookItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance,
                                        boost::shared_ptr<Player> player,
                                        std::vector<HtmlString>* lines,
                                        bool advanced)
{
    Item::appendHoverText(itemInstance, player, lines, advanced);
    ListTag* enchantments = getEnchantments(itemInstance);
    if (enchantments == NULL)
    {
        return;
    }
    for (int i = 0; i < enchantments->size(); ++i)
    {
        CompoundTag* entry = enchantments->getCompound(i);
        int id = entry->getShort(L"id");
        int level = entry->getShort(L"lvl");
        Enchantment* enchantment = Enchantment::byId(id);
        if (enchantment != NULL)
        {
            lines->push_back(enchantment->getFullname(level));
        }
    }
}

void EnchantedBookItem::addEnchantment(not_null_ptr<ItemInstance> itemInstance,
                                       EnchantmentInstance* instance)
{
    ListTag* enchantments = getEnchantments(itemInstance);
    for (int i = 0; i < enchantments->size(); ++i)
    {
        CompoundTag* entry = enchantments->getCompound(i);
        if (Enchantment::byId(entry->getShort(L"id")) == instance->enchantment)
        {
            if (instance->level > entry->getShort(L"lvl"))
            {
                entry->putShort(L"lvl", static_cast<short>(instance->level));
            }
            if (!itemInstance->hasTag())
            {
                itemInstance->setTag(new CompoundTag());
            }
            itemInstance->getTag()->put(L"StoredEnchantments", enchantments);
            return;
        }
    }
    CompoundTag* entry = new CompoundTag();
    entry->putShort(L"id", static_cast<short>(Enchantment::getId(instance->enchantment)));
    entry->putShort(L"lvl", static_cast<short>(instance->level));
    enchantments->add(entry);
    if (!itemInstance->hasTag())
    {
        itemInstance->setTag(new CompoundTag());
    }
    itemInstance->getTag()->put(L"StoredEnchantments", enchantments);
}

not_null_ptr<ItemInstance> EnchantedBookItem::createForEnchantment(EnchantmentInstance* instance)
{
    not_null_ptr<ItemInstance> item(new ItemInstance(Item::byId(403)));
    addEnchantment(item, instance);
    return item;
}

void EnchantedBookItem::createForEnchantment(Enchantment* enchantment,
                                             std::vector<not_null_ptr<ItemInstance> >* items)
{
    for (int level = enchantment->getMinLevel(); level <= enchantment->getMaxLevel(); ++level)
    {
        EnchantmentInstance instance(enchantment, level);
        items->push_back(createForEnchantment(&instance));
    }
}
