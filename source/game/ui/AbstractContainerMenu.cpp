#include "game/ui/AbstractContainerMenu.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"

AbstractContainerMenu::AbstractContainerMenu()
    : m_containerId(0), m_transactionId(0), m_quickcraftType(-1), m_quickcraftStatus(0), m_quickcraftActive(false), m_changed(false), m_data(0)
{
}

AbstractContainerMenu::~AbstractContainerMenu()
{
}

bool AbstractContainerMenu::isPauseScreen()
{
    return false;
}

int AbstractContainerMenu::addSlot(Slot* slot)
{
    m_slots.push_back(slot);
    m_items.push_back(ItemInstance::EMPTY);
    return (int)m_slots.size() - 1;
}

void AbstractContainerMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    m_listeners.push_back(listener);
    listener->refreshContainer(this, &m_items);
    broadcastChanges();
}

void AbstractContainerMenu::removeSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it = m_listeners.begin();
    while (it != m_listeners.end() && *it != listener) ++it;
    if (it != m_listeners.end()) m_listeners.erase(it);
}

AbstractContainerMenu::ItemList* AbstractContainerMenu::getItems() { return &m_items; }

void AbstractContainerMenu::sendData(int id, int value)
{
    std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it;
    for (it = m_listeners.begin(); it != m_listeners.end(); ++it) (*it)->setContainerData(this, id, value);
}

void AbstractContainerMenu::broadcastChanges()
{
    unsigned int i;
    for (i = 0; i < m_slots.size(); ++i) {
        not_null_ptr<ItemInstance> item = m_slots[i]->getItem();
        if (!ItemInstance::matches(m_items[i], item, true)) {
            m_items[i] = item;
            std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it;
            for (it = m_listeners.begin(); it != m_listeners.end(); ++it) (*it)->slotChanged(this, (int)i, item);
        }
    }
}

bool AbstractContainerMenu::needsRendered() { return m_changed; }
bool AbstractContainerMenu::clickMenuButton(boost::shared_ptr<Player>, int) { return false; }

Slot* AbstractContainerMenu::getSlotFor(boost::shared_ptr<Container> container, int index)
{
    std::vector<Slot*>::iterator it;
    for (it = m_slots.begin(); it != m_slots.end(); ++it) if ((*it)->isAt(container, index)) return *it;
    return NULL;
}

Slot* AbstractContainerMenu::getSlot(int index)
{
    return index >= 0 && index < (int)m_slots.size() ? m_slots[index] : NULL;
}

AbstractContainerMenu::ItemList AbstractContainerMenu::getAllItems(int start, int count)
{
    ItemList result;
    int end = start + count;
    int i;
    for (i = start; i < end && i < (int)m_slots.size(); ++i) result.push_back(m_slots[i]->getItem());
    return result;
}

AbstractContainerMenu::ItemList AbstractContainerMenu::takeAll(boost::shared_ptr<Player>, int start, int count, int)
{
    return getAllItems(start, count);
}

not_null_ptr<ItemInstance> AbstractContainerMenu::quickMoveStack(boost::shared_ptr<Player>, int) { return ItemInstance::EMPTY; }
int AbstractContainerMenu::getQuickcraftHeader(int flags) { return flags & 3; }

void AbstractContainerMenu::resetQuickCraft() { m_quickcraftStatus = 0; m_quickcraftSlots.clear(); }
int AbstractContainerMenu::getQuickcraftType(int flags) { return (flags >> 2) & 3; }
bool AbstractContainerMenu::isValidQuickcraftType(int type, boost::shared_ptr<Player>) { return type == 0 || type == 1 || type == 2; }

bool AbstractContainerMenu::canItemQuickReplace(Slot* slot, not_null_ptr<ItemInstance> item, bool stacking)
{
    if (!slot->hasItem()) return true;
    if (!stacking) return false;
    not_null_ptr<ItemInstance> oldItem = slot->getItem();
    return ItemInstance::matches(oldItem, item, true) && oldItem->getCount() < slot->getMaxStackSize(item);
}

bool AbstractContainerMenu::canDragTo(Slot*) { return true; }

int AbstractContainerMenu::getQuickCraftSlotCount(SlotSet* slots, int mode, not_null_ptr<ItemInstance> item, int slotCount)
{
    if (slots == NULL || slots->empty()) return 0;
    if (mode == 0) return item->getCount() / (int)slots->size();
    if (mode == 1) return 1;
    return item->getMaxStackSize();
}

not_null_ptr<ItemInstance> AbstractContainerMenu::clicked(int, int, ClickType::eClickType, boost::shared_ptr<Player>) { return ItemInstance::EMPTY; }
bool AbstractContainerMenu::mayCombine(Slot* slot, not_null_ptr<ItemInstance> item) { return slot != NULL && slot->mayCombine(item); }
bool AbstractContainerMenu::canTakeItemForPickAll(not_null_ptr<ItemInstance>, Slot*) { return true; }

void AbstractContainerMenu::clearContainer(boost::shared_ptr<Player>, Level*, boost::shared_ptr<Container> container)
{
    if (container) container->clearContent();
}

void AbstractContainerMenu::removed(boost::shared_ptr<Player> player)
{
    if (!player || !player->mInventory)
        return;

    not_null_ptr<ItemInstance> carried = player->mInventory->getCarried();
    if (!carried->isEmpty())
    {
        player->drop(carried, false);
        player->mInventory->setCarried(ItemInstance::EMPTY);
    }
}
void AbstractContainerMenu::slotsChanged() { broadcastChanges(); }

void AbstractContainerMenu::setItem(unsigned int index, not_null_ptr<ItemInstance> item)
{
    if (index < m_slots.size()) { m_items[index] = item; m_slots[index]->set(item); }
}

void AbstractContainerMenu::setAll(NonNullList<not_null_ptr<ItemInstance> > items)
{
    unsigned int i;
    for (i = 0; i < items.size() && i < m_slots.size(); ++i) setItem(i, items[i]);
}

void AbstractContainerMenu::setData(int, int) { }
int AbstractContainerMenu::backup(boost::shared_ptr<Inventory>) { return ++m_transactionId; }
bool AbstractContainerMenu::isSynched(boost::shared_ptr<Player> player) { return m_synched.find(player) != m_synched.end(); }
void AbstractContainerMenu::setSynched(boost::shared_ptr<Player> player, bool value) { if (value) m_synched.insert(player); else m_synched.erase(player); }
bool AbstractContainerMenu::stackAsdAsd(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second) { return ItemInstance::matches(first, second, true); }

int AbstractContainerMenu::moveItemStackTo(not_null_ptr<ItemInstance> item, int start, int end, bool reverse, bool, bool)
{
    bool moved = false;
    int i = reverse ? end - 1 : start;
    while (reverse ? i >= start : i < end) {
        Slot* slot = getSlot(i);
        if (slot != NULL && canItemQuickReplace(slot, item, true)) { slot->set(item); moved = true; }
        if (reverse) --i; else ++i;
    }
    return moved ? 1 : 0;
}

bool AbstractContainerMenu::isOverrideResultClick(int, int) { return false; }
int AbstractContainerMenu::getQuickcraftMask(int header, int type) { return (type & 3) << 2 | (header & 3); }

int AbstractContainerMenu::getRedstoneSignalFromContainer(boost::shared_ptr<Container> container)
{
    if (!container || container->getContainerSize() == 0) return 0;
    int filled = 0;
    unsigned int i;
    for (i = 0; i < container->getContainerSize(); ++i) if (!container->getItem(i)->isEmpty()) ++filled;
    return (filled * 15) / (int)container->getContainerSize();
}

int AbstractContainerMenu::getRedstoneSignalFromBlockEntity(boost::shared_ptr<BlockEntity> entity)
{
    if (!entity) return 0;
    boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(entity);
    return container ? getRedstoneSignalFromContainer(container) : 0;
}
bool AbstractContainerMenu::isValidIngredient(not_null_ptr<ItemInstance>, int) { return true; }
not_null_ptr<ItemInstance> AbstractContainerMenu::GetQuickCraftItem(int, int) { return ItemInstance::EMPTY; }
bool AbstractContainerMenu::IsSmallInventory()
{
    if (!g_pApp || !g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f))) return false;
    Minecraft* minecraft = Minecraft::GetInstance();
    return minecraft != NULL && minecraft->GetMiniGame() != NULL &&
           minecraft->GetMiniGame()->IsSmallInventoryEnabled();
}
bool AbstractContainerMenu::matches(AbstractContainerMenu* first, AbstractContainerMenu* second) { return first != NULL && second != NULL && first->m_slots.size() == second->m_slots.size(); }
bool AbstractContainerMenu::doQuickMoveLoop() { return true; }
