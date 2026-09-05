#include "game/item/Inventory.hpp"

#include <algorithm>
#include <vector>

#include "game/core/GenericStats.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

Inventory::Inventory(Player* player)
    : m_items(NonNullList<not_null_ptr<ItemInstance> >::withSize(36, ItemInstance::EMPTY)),
      m_armor(NonNullList<not_null_ptr<ItemInstance> >::withSize(4, ItemInstance::EMPTY)),
      m_offhand(NonNullList<not_null_ptr<ItemInstance> >::withSize(1, ItemInstance::EMPTY)),
      m_hasPickedItem(false),
      m_selected(0),
      m_player(player),
      m_auxiliaryItem(ItemInstance::EMPTY),
      m_carried(ItemInstance::EMPTY),
      m_timesChanged(0)
{
    m_compartments[0] = m_items;
    m_compartments[1] = m_armor;
    m_compartments[2] = m_offhand;
}

Inventory::~Inventory()
{
}

std::wstring Inventory::getName()
{
    return std::wstring(L10N::GetString(0x14630b9e));
}

std::wstring Inventory::getCustomName()
{
    return std::wstring();
}

bool Inventory::hasCustomName()
{
    return false;
}

std::wstring Inventory::getDisplayName()
{
    return hasCustomName() ? getCustomName() : getName();
}

bool Inventory::isHotbarSlot(int slot)
{
    return slot >= 0 && slot < 9;
}

not_null_ptr<ItemInstance> Inventory::getSelected()
{
    return isHotbarSlot(m_selected) ? m_items[m_selected] : ItemInstance::EMPTY;
}

int Inventory::getSelectionSize()
{
    return 9;
}

bool Inventory::isSameItem(not_null_ptr<ItemInstance> first,
                           not_null_ptr<ItemInstance> second,
                           bool ignoreTags)
{
    return first->getItem() == second->getItem()
        && (!first->isStackedByData() || first->getAuxValue() == second->getAuxValue())
        && (ignoreTags || ItemInstance::tagMatches(first, second));
}

bool Inventory::hasRemainingSpaceForItem(not_null_ptr<ItemInstance> first,
                                         not_null_ptr<ItemInstance> second,
                                         bool ignoreTags)
{
    return !first->isEmpty()
        && isSameItem(first, second, ignoreTags)
        && first->isStackable()
        && first->getCount() < first->getMaxStackSize()
        && first->getCount() < getMaxStackSize();
}

int Inventory::getFreeSlot()
{
    unsigned int size = m_items.size();
    if (Minecraft::InMiniGame(eMiniGameId_Any, true)
        && Minecraft::GetInstance()->GetMiniGame()->IsSmallInventoryEnabled())
    {
        size = 9;
    }
    for (unsigned int slot = 0; slot < size; ++slot)
    {
        if (m_items[slot]->isEmpty())
        {
            return static_cast<int>(slot);
        }
    }
    return -1;
}

bool Inventory::hasPickedItem()
{
    bool result = m_hasPickedItem;
    m_hasPickedItem = false;
    return result;
}

int Inventory::findSlotMatchingItem(not_null_ptr<ItemInstance> item, bool ignoreTags)
{
    for (unsigned int slot = 0; slot < m_items.size(); ++slot)
    {
        if (!m_items[slot]->isEmpty() && isSameItem(item, m_items[slot], ignoreTags))
        {
            return static_cast<int>(slot);
        }
    }
    return -1;
}

int Inventory::getSuitableHotbarSlot()
{
    for (int offset = 0; offset < 9; ++offset)
    {
        int slot = (m_selected + offset) % 9;
        if (m_items[slot]->isEmpty())
        {
            return slot;
        }
    }
    for (int offset = 0; offset < 9; ++offset)
    {
        int slot = (m_selected + offset) % 9;
        if (!m_items[slot]->isEnchanted())
        {
            return slot;
        }
    }
    return m_selected;
}

void Inventory::pickSlot(int slot)
{
    int selected = getSuitableHotbarSlot();
    m_selected = selected;
    std::swap(m_items[selected], m_items[slot]);
}

void Inventory::setPickedItem(not_null_ptr<ItemInstance> item)
{
    int slot = findSlotMatchingItem(item, false);
    if (isHotbarSlot(slot))
    {
        m_selected = slot;
        m_hasPickedItem = true;
    }
    else if (slot != -1)
    {
        pickSlot(slot);
    }
    else if (m_player->mAbilities.mInstabuild)
    {
        m_selected = getSuitableHotbarSlot();
        if (!m_items[m_selected]->isEmpty())
        {
            int freeSlot = getFreeSlot();
            if (freeSlot != -1)
            {
                m_items[freeSlot] = m_items[m_selected];
            }
        }
        m_items[m_selected] = item;
    }
}

int Inventory::findSlotMatchingUnusedItem(not_null_ptr<ItemInstance> item)
{
    for (unsigned int slot = 0; slot < m_items.size(); ++slot)
    {
        not_null_ptr<ItemInstance> current = m_items[slot];
        if (!current->isEmpty()
            && isSameItem(item, current, true)
            && !current->isDamaged()
            && !current->isEnchanted()
            && !current->hasCustomHoverName())
        {
            return static_cast<int>(slot);
        }
    }
    return -1;
}

void Inventory::swapPaint(int direction)
{
    direction = direction < 0 ? -1 : direction > 0 ? 1 : 0;
    m_selected -= direction;
    while (m_selected < 0)
    {
        m_selected += 9;
    }
    while (m_selected >= 9)
    {
        m_selected -= 9;
    }
}

int Inventory::clearInventory(Item* item, int auxValue, int count, CompoundTag* tag)
{
    int removed = 0;
    for (unsigned int slot = 0; slot < getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> current = getItem(slot);
        if (current->isEmpty()
            || (item != NULL && current->getItem() != item)
            || (auxValue >= 0 && current->getAuxValue() != auxValue))
        {
            continue;
        }
        int amount = count < 1 ? current->getCount()
                               : std::min(current->getCount(), count - removed);
        removed += amount;
        if (count != 0)
        {
            current->shrink(amount);
            if (current->isEmpty())
            {
                setItem(slot, ItemInstance::EMPTY);
            }
            if (removed >= count)
            {
                return removed;
            }
        }
    }

    if (!m_carried->isEmpty()
        && (item == NULL || m_carried->getItem() == item)
        && (auxValue < 0 || m_carried->getAuxValue() == auxValue))
    {
        int amount = count < 1 ? m_carried->getCount()
                               : std::min(m_carried->getCount(), count - removed);
        removed += amount;
        if (count != 0)
        {
            m_carried->shrink(amount);
            if (m_carried->isEmpty())
            {
                m_carried = ItemInstance::EMPTY;
            }
        }
    }
    return removed;
}

int Inventory::getSlotWithRemainingSpace(not_null_ptr<ItemInstance> item)
{
    if (hasRemainingSpaceForItem(getItem(m_selected), item, false))
    {
        return m_selected;
    }
    if (hasRemainingSpaceForItem(getItem(40), item, false))
    {
        return 40;
    }
    for (unsigned int slot = 0; slot < m_items.size(); ++slot)
    {
        if (hasRemainingSpaceForItem(m_items[slot], item, false))
        {
            return static_cast<int>(slot);
        }
    }
    return -1;
}

int Inventory::addResource(int slot, not_null_ptr<ItemInstance> item)
{
    Item* itemType = item->getItem();
    int remaining = item->getCount();
    not_null_ptr<ItemInstance> current = getItem(slot);
    if (current->isEmpty())
    {
        current = not_null_ptr<ItemInstance>(new ItemInstance(itemType, 0, item->getAuxValue()));
        if (item->hasTag())
        {
            current->setTag(item->getTag()->copy());
        }
        setItem(slot, current);
    }

    int amount = std::min(remaining, current->getMaxStackSize() - current->getCount());
    amount = std::min(amount, getMaxStackSize() - current->getCount());
    if (amount == 0)
    {
        return remaining;
    }
    current->grow(amount);
    current->setPopTime(5);
    return remaining - amount;
}

int Inventory::addResource(not_null_ptr<ItemInstance> item)
{
    if (m_player->GetGameMode() != NULL)
    {
        m_player->GetGameMode()->RecordItem(item);
    }
    int slot = getSlotWithRemainingSpace(item);
    if (slot == -1)
    {
        slot = getFreeSlot();
    }
    return slot == -1 ? item->getCount() : addResource(slot, item);
}

void Inventory::tick()
{
    boost::shared_ptr<Entity> owner = m_player->shared_from_this();
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        for (unsigned int slot = 0; slot < m_compartments[compartment].size(); ++slot)
        {
            not_null_ptr<ItemInstance> item = m_compartments[compartment][slot];
            if (!item->isEmpty())
            {
                item->inventoryTick(m_player->mLevel, owner, slot, m_selected == static_cast<int>(slot));
            }
        }
    }
}

bool Inventory::add(int slot, not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty())
    {
        return false;
    }

    not_null_ptr<ItemInstance> original = item->copy();
    if (item->isDamaged())
    {
        if (slot == -1)
        {
            slot = getFreeSlot();
        }
        if (slot < 0)
        {
            if (m_player->mAbilities.mInstabuild)
            {
                item->setCount(0);
                return true;
            }
            return false;
        }
        not_null_ptr<ItemInstance> stored = item->copy();
        stored->setPopTime(5);
        m_items[slot] = stored;
        m_player->handleCollectItem(stored);
        m_player->awardStat(GenericStats::itemsCollected(original),
                            GenericStats::param_itemsCollected(original));
        if (original->getItem() == Items::leather)
        {
            m_player->awardStat(GenericStats::killCow(), GenericStats::param_noArgs());
        }
        item->setCount(0);
        return true;
    }

    int originalCount = item->getCount();
    int previousCount;
    do
    {
        previousCount = item->getCount();
        item->setCount(slot == -1 ? addResource(item) : addResource(slot, item));
    }
    while (!item->isEmpty() && item->getCount() < previousCount);

    if (item->getCount() == originalCount && m_player->mAbilities.mInstabuild)
    {
        item->setCount(0);
        return true;
    }
    int collected = originalCount - item->getCount();
    if (collected > 0)
    {
        m_player->awardStat(GenericStats::itemsCollected(original),
                            GenericStats::param_itemsCollected(
                                original->getItem(), original->getAuxValue(), collected));
        if (original->getItem() == Items::leather)
        {
            m_player->awardStat(GenericStats::killCow(), GenericStats::param_noArgs());
        }
    }
    return collected > 0;
}

bool Inventory::add(not_null_ptr<ItemInstance> item)
{
    return add(-1, item);
}

void Inventory::placeItemBackInInventory(Level* level, not_null_ptr<ItemInstance> item)
{
    if (level->mIsClientSide)
    {
        return;
    }
    while (!item->isEmpty())
    {
        int slot = getSlotWithRemainingSpace(item);
        if (slot == -1)
        {
            slot = getFreeSlot();
        }
        if (slot == -1)
        {
            m_player->drop(item, false);
            return;
        }

        not_null_ptr<ItemInstance> current = getItem(slot);
        int currentCount = current->isEmpty() ? 0 : current->getCount();
        int room = current->isEmpty() ? item->getMaxStackSize()
                                      : current->getMaxStackSize() - currentCount;
        room = std::min(room, getMaxStackSize() - currentCount);
        not_null_ptr<ItemInstance> part(new ItemInstance(item->split(std::min(room, item->getCount()))));
        if (!add(slot, part))
        {
            m_player->drop(item, false);
            return;
        }
    }
}

not_null_ptr<ItemInstance> Inventory::removeItem(unsigned int slot, int count)
{
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        if (slot < m_compartments[compartment].size())
        {
            if (!m_compartments[compartment][slot]->isEmpty())
            {
                return ContainerHelper::removeItem(m_compartments[compartment], slot, count);
            }
            return ItemInstance::EMPTY;
        }
        slot -= m_compartments[compartment].size();
    }
    return ItemInstance::EMPTY;
}

void Inventory::removeItem(not_null_ptr<ItemInstance> item)
{
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        for (unsigned int slot = 0; slot < m_compartments[compartment].size(); ++slot)
        {
            if (m_compartments[compartment][slot].get() == item.get())
            {
                m_compartments[compartment][slot] = ItemInstance::EMPTY;
                break;
            }
        }
    }
}

not_null_ptr<ItemInstance> Inventory::removeItemNoUpdate(int slot)
{
    if (slot < 0)
    {
        return ItemInstance::EMPTY;
    }
    unsigned int index = static_cast<unsigned int>(slot);
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        if (index < m_compartments[compartment].size())
        {
            return m_compartments[compartment][index]->isEmpty()
                ? ItemInstance::EMPTY
                : ContainerHelper::takeItem(m_compartments[compartment], index);
        }
        index -= m_compartments[compartment].size();
    }
    return ItemInstance::EMPTY;
}

void Inventory::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        if (slot < m_compartments[compartment].size())
        {
            if (compartment == 1 && !item->isEmpty() && m_player->GetType() == ENTITY_TYPE_LOCAL_PLAYER)
            {
                m_player->OnEquipArmor(item);
            }
            m_compartments[compartment][slot] = item;
            m_player->handleCollectItem(item);
            return;
        }
        slot -= m_compartments[compartment].size();
    }
}

float Inventory::getDestroySpeed(const BlockState* block)
{
    not_null_ptr<ItemInstance> selected = getItem(m_selected);
    return selected->isEmpty() ? 1.0f : selected->getDestroySpeed(block);
}

ListTag* Inventory::save(ListTag* tag)
{
    for (unsigned int slot = 0; slot < m_items.size(); ++slot)
    {
        if (!m_items[slot]->isEmpty())
        {
            CompoundTag* itemTag = new CompoundTag();
            itemTag->putByte(L"Slot", static_cast<unsigned char>(slot));
            m_items[slot]->save(itemTag);
            tag->add(itemTag);
        }
    }
    for (unsigned int slot = 0; slot < m_armor.size(); ++slot)
    {
        if (!m_armor[slot]->isEmpty())
        {
            CompoundTag* itemTag = new CompoundTag();
            itemTag->putByte(L"Slot", static_cast<unsigned char>(slot + 100));
            m_armor[slot]->save(itemTag);
            tag->add(itemTag);
        }
    }
    for (unsigned int slot = 0; slot < m_offhand.size(); ++slot)
    {
        if (!m_offhand[slot]->isEmpty())
        {
            CompoundTag* itemTag = new CompoundTag();
            itemTag->putByte(L"Slot", static_cast<unsigned char>(slot + 150));
            m_offhand[slot]->save(itemTag);
            tag->add(itemTag);
        }
    }
    return tag;
}

void Inventory::load(ListTag* tag)
{
    clearContent();
    for (int index = 0; index < tag->size(); ++index)
    {
        CompoundTag* itemTag = tag->getCompound(index);
        unsigned int slot = itemTag->getByte(L"Slot");
        not_null_ptr<ItemInstance> item(new ItemInstance(itemTag));
        if (item->isEmpty())
        {
            continue;
        }
        if (slot < m_items.size())
        {
            m_items[slot] = item;
        }
        else if (slot >= 100 && slot < 100 + m_armor.size())
        {
            m_armor[slot - 100] = item;
        }
        else if (slot >= 150 && slot < 150 + m_offhand.size())
        {
            m_offhand[slot - 150] = item;
        }
    }
}

unsigned int Inventory::getContainerSize()
{
    return m_items.size() + m_armor.size() + m_offhand.size();
}

bool Inventory::isEmpty()
{
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        for (unsigned int slot = 0; slot < m_compartments[compartment].size(); ++slot)
        {
            if (!m_compartments[compartment][slot]->isEmpty())
            {
                return false;
            }
        }
    }
    return true;
}

not_null_ptr<ItemInstance> Inventory::getItem(unsigned int slot)
{
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        if (slot < m_compartments[compartment].size())
        {
            return m_compartments[compartment][slot];
        }
        slot -= m_compartments[compartment].size();
    }
    return ItemInstance::EMPTY;
}

int Inventory::getMaxStackSize()
{
    return 64;
}

bool Inventory::canDestroy(const BlockState* block)
{
    if (block->getBlock()->mMaterial->isAlwaysDestroyable())
    {
        return true;
    }
    not_null_ptr<ItemInstance> selected = getItem(m_selected);
    return !selected->isEmpty() && selected->canDestroySpecial(block);
}

not_null_ptr<ItemInstance> Inventory::getArmor(int slot)
{
    return m_armor[slot];
}

void Inventory::hurtArmor(float damage)
{
    int amount = static_cast<int>(damage / 4.0f);
    if (amount < 1)
    {
        amount = 1;
    }
    boost::shared_ptr<LivingEntity> owner =
        boost::static_pointer_cast<LivingEntity>(m_player->shared_from_this());
    for (unsigned int slot = 0; slot < m_armor.size(); ++slot)
    {
        if (!m_armor[slot]->isEmpty()
            && dynamic_cast<ArmorItem*>(m_armor[slot]->getItem()) != NULL)
        {
            m_armor[slot]->hurtAndBreak(amount, owner);
            if (m_armor[slot]->getCount() == 0)
            {
                m_armor[slot] = ItemInstance::EMPTY;
            }
        }
    }
}

void Inventory::dropAll()
{
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        for (unsigned int slot = 0; slot < m_compartments[compartment].size(); ++slot)
        {
            if (!m_compartments[compartment][slot]->isEmpty())
            {
                m_player->drop(m_compartments[compartment][slot], true, false);
                m_compartments[compartment][slot] = ItemInstance::EMPTY;
            }
        }
    }
}

void Inventory::setChanged()
{
    ++m_timesChanged;
}

void Inventory::setCarried(not_null_ptr<ItemInstance> item)
{
    m_carried = item;
    m_player->handleCollectItem(item);
}

not_null_ptr<ItemInstance> Inventory::getCarried()
{
    return m_carried;
}

bool Inventory::stillValid(boost::shared_ptr<Player> player)
{
    if (m_player->mRemoved)
    {
        return false;
    }
    BlockPos position(m_player->getPos(1.0f));
    return player->distanceAsIfHitscanned(position, *player->mBoundingBox) <= 30.25;
}

void Inventory::startOpen(boost::shared_ptr<Player> player)
{
}

void Inventory::stopOpen(boost::shared_ptr<Player> player)
{
}

bool Inventory::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    return true;
}

void Inventory::replaceWith(boost::shared_ptr<Inventory> inventory)
{
    for (unsigned int slot = 0; slot < getContainerSize(); ++slot)
    {
        setItem(slot, inventory->getItem(slot));
    }
    m_selected = inventory->m_selected;
}

void Inventory::replaceWith_NONRECURSIVE(boost::shared_ptr<Inventory> inventory)
{
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        unsigned int size = std::min(m_compartments[compartment].size(),
                                     inventory->m_compartments[compartment].size());
        for (unsigned int slot = 0; slot < size; ++slot)
        {
            m_compartments[compartment][slot] =
                inventory->m_compartments[compartment][slot]->copy();
        }
    }
}

int Inventory::getContainerData(int id)
{
    return 0;
}

void Inventory::setContainerData(int id, int value)
{
}

int Inventory::getContainerDataCount()
{
    return 0;
}

void Inventory::clearContent()
{
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        for (unsigned int slot = 0; slot < m_compartments[compartment].size(); ++slot)
        {
            m_compartments[compartment][slot] = ItemInstance::EMPTY;
        }
    }
}

int Inventory::countMatches(not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty())
    {
        return 0;
    }
    int count = 0;
    for (unsigned int slot = 0; slot < m_items.size(); ++slot)
    {
        if (!m_items[slot]->isEmpty() && m_items[slot]->sameItem(item))
        {
            count += m_items[slot]->getCount();
        }
    }
    return count;
}

void Inventory::OnRemove(int slot)
{
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || !isHotbarSlot(slot))
    {
        return;
    }
    for (unsigned int source = 9; source < m_items.size(); ++source)
    {
        if (!m_items[source]->isEmpty() && !m_items[source]->getItem()->PreferInventory())
        {
            m_items[slot] = m_items[source];
            m_items[source] = ItemInstance::EMPTY;
            return;
        }
    }
}

void Inventory::PrintContents()
{
    for (unsigned int slot = 0; slot < m_items.size(); ++slot)
    {
        if (!m_items[slot]->isEmpty())
        {
            m_items[slot]->toString();
        }
    }
}

int Inventory::getSlot(Item* item, int auxValue, CompoundTag* tag)
{
    std::vector<std::wstring> exceptions;
    exceptions.push_back(L"display");
    exceptions.push_back(L"Name");
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        for (unsigned int slot = 0; slot < m_compartments[compartment].size(); ++slot)
        {
            not_null_ptr<ItemInstance> current = m_compartments[compartment][slot];
            if (current->getItem() != item
                || (auxValue >= 0 && current->getAuxValue() != auxValue))
            {
                continue;
            }
            if (tag == NULL)
            {
                return static_cast<int>(slot);
            }
            if (current->getTag() != NULL
                && tag->equalsWithException(current->getTag(), &exceptions))
            {
                return static_cast<int>(slot);
            }
        }
    }
    return -1;
}

bool Inventory::removeResource(Item* item, bool flag)
{
    int slot = getSlot(item, -1, NULL);
    if (slot < 0)
    {
        return false;
    }
    removeItem(slot, 1);
    return true;
}

bool Inventory::removeResource(Item* item, int auxValue, bool flag)
{
    int slot = getSlot(item, auxValue, NULL);
    if (slot < 0)
    {
        return false;
    }
    removeItem(slot, 1);
    return true;
}

void Inventory::removeResources(not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty())
    {
        return;
    }
    int remaining = item->getCount();
    while (remaining > 0)
    {
        int slot = findSlotMatchingItem(item, true);
        if (slot < 0)
        {
            break;
        }
        not_null_ptr<ItemInstance> removed = removeItem(slot, remaining);
        remaining -= removed->getCount();
    }
}

int Inventory::removeResourcesWithTags(not_null_ptr<ItemInstance> item)
{
    if (!item->isEmpty())
    {
        int slot = findSlotMatchingItem(item, false);
        if (slot >= 0)
        {
            removeItem(slot, item->getCount());
            return 1;
        }
    }
    return 0;
}

not_null_ptr<ItemInstance> Inventory::getResourceItem(Item* item)
{
    int slot = getSlot(item, -1, NULL);
    return slot < 0 ? ItemInstance::EMPTY : getItem(slot);
}

not_null_ptr<ItemInstance> Inventory::getResourceItem(Item* item, int auxValue)
{
    int slot = getSlot(item, auxValue, NULL);
    return slot < 0 ? ItemInstance::EMPTY : getItem(slot);
}

not_null_ptr<ItemInstance> Inventory::getResourceItem(Item* item, int auxValue, CompoundTag* tag)
{
    int slot = getSlot(item, auxValue, tag);
    return slot < 0 ? ItemInstance::EMPTY : getItem(slot);
}

bool Inventory::hasResource(Item* item)
{
    return getSlot(item, -1, NULL) >= 0;
}
