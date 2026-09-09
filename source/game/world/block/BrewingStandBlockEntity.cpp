#include "game/world/block/BrewingStandBlockEntity.hpp"

#include "game/core/Direction.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/core/DataWalker.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/item/PotionBrewing.hpp"
#include "game/ui/BrewingStandMenu.hpp"

BrewingStandBlockEntity::BrewingStandBlockEntity()
    : mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(5, ItemInstance::EMPTY)),
      mBrewTime(0),
      mUnknown0(0),
      mUnknown1(0),
      mUnknown2(0),
      mFuel(0),
      mCustomName(new CensoredString())
{
}

BrewingStandBlockEntity::~BrewingStandBlockEntity()
{
}

std::wstring BrewingStandBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring(L"container.brewing_stand");
}

std::wstring BrewingStandBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring();
}

bool BrewingStandBlockEntity::hasCustomName()
{
    return mCustomName && mCustomName->checkParentalFilter() && !mCustomName->raw().empty();
}

unsigned int BrewingStandBlockEntity::getContainerSize()
{
    return mItems.size();
}

bool BrewingStandBlockEntity::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        if (!mItems[i]->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> BrewingStandBlockEntity::getItem(unsigned int slot)
{
    if (slot >= mItems.size())
    {
        return ItemInstance::EMPTY;
    }
    return mItems[slot];
}

not_null_ptr<ItemInstance> BrewingStandBlockEntity::removeItem(unsigned int slot, int count)
{
    if (slot >= mItems.size())
    {
        return ItemInstance::EMPTY;
    }
    return ContainerHelper::removeItem(mItems, slot, count);
}

not_null_ptr<ItemInstance> BrewingStandBlockEntity::removeItemNoUpdate(int slot)
{
    if (slot < 0 || (unsigned int)slot >= mItems.size())
    {
        return ItemInstance::EMPTY;
    }
    return ContainerHelper::takeItem(mItems, (unsigned int)slot);
}

void BrewingStandBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance)
{
    if (slot < mItems.size())
    {
        mItems[slot] = itemInstance;
    }
}

int BrewingStandBlockEntity::getMaxStackSize()
{
    return 64;
}

bool BrewingStandBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    if (player.get() == NULL)
    {
        return false;
    }
    return !isRemoved();
}

void BrewingStandBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
}

void BrewingStandBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
}

bool BrewingStandBlockEntity::canPlaceItem(int slot, not_null_ptr<ItemInstance> itemInstance)
{
    if (slot == 3)
    {
        return PotionBrewing::isIngredient(itemInstance);
    }
    if (slot == 4)
    {
        return itemInstance->getItem() == NULL || itemInstance->isEmpty();
    }
    if (slot < 0 || slot > 2)
    {
        return false;
    }
    return !itemInstance->isEmpty() && PotionBrewing::isContainerIngredient(itemInstance);
}

int BrewingStandBlockEntity::getContainerData(int id)
{
    if (id == 0)
    {
        return mBrewTime;
    }
    if (id == 1)
    {
        return mFuel;
    }
    return 0;
}

void BrewingStandBlockEntity::setContainerData(int id, int value)
{
    if (id == 0)
    {
        mBrewTime = value;
    }
    else if (id == 1)
    {
        mFuel = value;
    }
}

int BrewingStandBlockEntity::getContainerDataCount()
{
    return 2;
}

void BrewingStandBlockEntity::clearContent()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        mItems[i] = ItemInstance::EMPTY;
    }
}

AbstractContainerMenu* BrewingStandBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player)
{
    (void)player;
    boost::shared_ptr<Container> self = boost::dynamic_pointer_cast<Container>(shared_from_this());
    return new BrewingStandMenu(inventory, self);
}

int BrewingStandBlockEntity::getTypeId(bool large)
{
    return 5;
}

InteractionObject::NameData BrewingStandBlockEntity::GetNetworkNameData()
{
    return InteractionObject::NameData(getName(), mCustomName ? mCustomName->raw() : std::wstring(),
                                       mCustomName ? mCustomName->checkParentalFilter() != 0 : false,
                                       false);
}

arrayWithLength<int> BrewingStandBlockEntity::getSlotsForFace(const Direction* face)
{
    if (face == NULL)
    {
        return arrayWithLength<int>(3, true);
    }
    return arrayWithLength<int>(3, true);
}

bool BrewingStandBlockEntity::canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> itemInstance, const Direction* face)
{
    return canPlaceItem(slot, itemInstance);
}

bool BrewingStandBlockEntity::canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> itemInstance, const Direction* face)
{
    if (slot == 3)
    {
        return itemInstance->getItem() != NULL;
    }
    return true;
}

void BrewingStandBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal)
{
    mCustomName->set(name, uuid, chatCensored, uuidLocal);
}

void BrewingStandBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    if (fixer != NULL)
    {
        fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity, (DataWalker*)new ItemListDataWalker(0, L"Items"));
    }
}

bool BrewingStandBlockEntity::isBrewable()
{
    if (mItems[3]->isEmpty() || !PotionBrewing::isIngredient(mItems[3]))
    {
        return false;
    }
    for (unsigned int i = 0; i < 3; ++i)
    {
        if (!mItems[i]->isEmpty() && PotionBrewing::hasMix(mItems[i], mItems[3]))
        {
            return true;
        }
    }
    return false;
}

arrayWithLength<unsigned char> BrewingStandBlockEntity::getPotionBits()
{
    arrayWithLength<unsigned char> bits(3, true);
    for (unsigned int i = 0; i < 3; ++i)
    {
        bits.data[i] = mItems[i]->isEmpty() ? 0 : 1;
    }
    return bits;
}

void BrewingStandBlockEntity::doBrew()
{
    if (!isBrewable())
    {
        return;
    }
    for (unsigned int i = 0; i < 3; ++i)
    {
        PotionBrewing::mix(mItems[i], mItems[3]);
    }
    mItems[3]->shrink(1);
    if (mItems[3]->isEmpty())
    {
        mItems[3] = ItemInstance::EMPTY;
    }
}

void BrewingStandBlockEntity::tick()
{
    bool brewable = isBrewable();
    if (mBrewTime > 0)
    {
        --mBrewTime;
        if (mBrewTime == 0 && brewable)
        {
            doBrew();
        }
    }
    else if (brewable && mFuel > 0)
    {
        --mFuel;
        mBrewTime = 400;
    }
}

void BrewingStandBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mCustomName->loadFromTag(tag);
    ContainerHelper::loadAllItems(tag, mItems);
    mBrewTime = tag->getShort(L"BrewTime");
    mFuel = tag->getByte(L"Fuel");
}

CompoundTag* BrewingStandBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mCustomName->saveToTag(tag);
    ContainerHelper::saveAllItems(tag, mItems);
    tag->putShort(L"BrewTime", (short)mBrewTime);
    tag->putByte(L"Fuel", (unsigned char)mFuel);
    return tag;
}

boost::shared_ptr<BlockEntity> BrewingStandBlockEntity::clone()
{
    boost::shared_ptr<BrewingStandBlockEntity> copy(new BrewingStandBlockEntity());
    copy->mItems = mItems;
    copy->mBrewTime = mBrewTime;
    copy->mFuel = mFuel;
    return copy;
}

std::wstring BrewingStandBlockEntity::getDisplayName()
{
    return LockableContainerBlockEntity::getDisplayName();
}

void BrewingStandBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

BlockEntity* BrewingStandBlockEntity::create()
{
    return new BrewingStandBlockEntity();
}

unsigned long long BrewingStandBlockEntity::GetType()
{
    return 0x0000000001E00203ULL;
}
