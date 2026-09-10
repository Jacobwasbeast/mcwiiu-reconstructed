#include "game/world/block/FurnaceBlockEntity.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/FurnaceRecipes.hpp"
#include "game/core/FurnaceFuelSlot.hpp"
#include "game/ui/FurnaceMenu.hpp"

FurnaceBlockEntity::FurnaceBlockEntity()
    : LockableContainerBlockEntity(), WorldlyContainer(),
      mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(3, not_null_ptr<ItemInstance>())),
      mOpeners(), mOpenersThisTick(), mLitTime(0), mLitDuration(0),
      mCookingProgress(0), mCookingTotalTime(0),
      mName(new CensoredString()), mCharcoalUsed(false)
{
}

FurnaceBlockEntity::~FurnaceBlockEntity()
{
}

BlockEntity* FurnaceBlockEntity::create()
{
    return new FurnaceBlockEntity();
}

std::wstring FurnaceBlockEntity::getName()
{
    return mName != NULL ? mName->get() : std::wstring();
}

std::wstring FurnaceBlockEntity::getCustomName()
{
    return mName != NULL ? mName->get() : std::wstring();
}

bool FurnaceBlockEntity::hasCustomName()
{
    return mName != NULL && mName->checkParentalFilter() != 0 && !mName->raw().empty();
}

unsigned int FurnaceBlockEntity::getContainerSize()
{
    return mItems.size();
}

bool FurnaceBlockEntity::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        if (!mItems[i]->isEmpty()) return false;
    }
    return true;
}

not_null_ptr<ItemInstance> FurnaceBlockEntity::getItem(unsigned int slot)
{
    return mItems[slot];
}

not_null_ptr<ItemInstance> FurnaceBlockEntity::removeItem(unsigned int slot, int count)
{
    return ContainerHelper::removeItem(mItems, slot, count);
}

not_null_ptr<ItemInstance> FurnaceBlockEntity::removeItemNoUpdate(int slot)
{
    return ContainerHelper::takeItem(mItems, slot);
}

int FurnaceBlockEntity::getTotalCookTime(not_null_ptr<ItemInstance> item)
{
    return 200;
}

void FurnaceBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    bool same = !item->isEmpty() && !mItems[slot]->isEmpty() && mItems[slot]->sameItem(item) && ItemInstance::tagMatches(mItems[slot], item);
    mItems[slot] = item;
    if (!item->isEmpty() && item->getCount() > getMaxStackSize()) item->setCount(getMaxStackSize());
    if (slot == 0 && !same)
    {
        mCookingTotalTime = getTotalCookTime(item);
        mCookingProgress = 0;
    }
    setChanged();
}

int FurnaceBlockEntity::getMaxStackSize()
{
    return 64;
}

void FurnaceBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

bool FurnaceBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    if (player.get() == NULL || mLevel == NULL) return false;
    Entity* entity = (Entity*)player.get();
    return entity->distanceAsIfHitscanned(mPos, *entity->mBoundingBox) <= 64.0;
}

void FurnaceBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
    mOpeners.insert(player);
}

void FurnaceBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
    mOpeners.erase(player);
}

int FurnaceBlockEntity::getBurnDuration(not_null_ptr<ItemInstance> item)
{
    if (item == NULL || item->isEmpty()) return 0;
    return 0;
}

bool FurnaceBlockEntity::isFuel(not_null_ptr<ItemInstance> item)
{
    return getBurnDuration(item) > 0;
}

bool FurnaceBlockEntity::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    if (slot == 2) return false;
    if (slot != 1) return true;
    return isFuel(item) || (FurnaceFuelSlot::isBucket(item) && mItems[1]->getItem() == NULL);
}

int FurnaceBlockEntity::getContainerData(int id)
{
    if (id == 0) return mLitTime;
    if (id == 1) return mLitDuration;
    if (id == 2) return mCookingProgress;
    if (id == 3) return mCookingTotalTime;
    if (id == 4) return mCharcoalUsed ? 1 : 0;
    return 0;
}

void FurnaceBlockEntity::setContainerData(int id, int value)
{
    if (id == 0) mLitTime = value;
    else if (id == 1) mLitDuration = value;
    else if (id == 2) mCookingProgress = value;
    else if (id == 3) mCookingTotalTime = value;
}

int FurnaceBlockEntity::getContainerDataCount()
{
    return 6;
}

void FurnaceBlockEntity::clearContent()
{
    for (unsigned int i = 0; i < mItems.size(); ++i) mItems[i] = not_null_ptr<ItemInstance>();
}

AbstractContainerMenu* FurnaceBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player)
{
    return new FurnaceMenu(inventory, boost::static_pointer_cast<FurnaceBlockEntity>(shared_from_this()));
}

int FurnaceBlockEntity::getTypeId(bool large)
{
    return 2;
}

InteractionObject::NameData FurnaceBlockEntity::GetNetworkNameData()
{
    return InteractionObject::NameData(getName(), std::wstring(), false, false);
}

std::wstring FurnaceBlockEntity::getDisplayName()
{
    return getName();
}

arrayWithLength<int> FurnaceBlockEntity::getSlotsForFace(const Direction* face)
{
    if (face == Direction::UP)
    {
        arrayWithLength<int> slots(1, true);
        slots.data[0] = 0;
        return slots;
    }
    if (face == Direction::DOWN)
    {
        arrayWithLength<int> slots(2, true);
        slots.data[0] = 2;
        slots.data[1] = 1;
        return slots;
    }
    arrayWithLength<int> slots(1, true);
    slots.data[0] = 1;
    return slots;
}

bool FurnaceBlockEntity::canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face)
{
    return canPlaceItem(slot, item);
}

bool FurnaceBlockEntity::canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face)
{
    return !(face == Direction::DOWN && slot == 1) || item->getItem() == NULL;
}

void FurnaceBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal)
{
    if (mName != NULL) mName->set(name, uuid, chatCensored, uuidLocal);
}

void FurnaceBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemListDataWalker(0x1e00202ULL, L"Items"));
}

void FurnaceBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    ContainerHelper::loadAllItems(tag, mItems);
    mLitTime = tag->getShort(L"BurnTime");
    mCookingProgress = tag->getShort(L"CookTime");
    mCookingTotalTime = tag->getShort(L"CookTimeTotal");
    mLitDuration = mLitTime;
    if (!mItems[0]->isEmpty()) mLitDuration = mLitTime > getBurnDuration(mItems[0]) ? mLitTime : getBurnDuration(mItems[0]);
    if (mName != NULL) mName->loadFromTag(tag);
    mCharcoalUsed = tag->getBoolean(L"CharcoalUsed");
}

CompoundTag* FurnaceBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putShort(L"BurnTime", (short)mLitTime);
    tag->putShort(L"CookTime", (short)mCookingProgress);
    tag->putShort(L"CookTimeTotal", (short)mCookingTotalTime);
    ContainerHelper::saveAllItems(tag, mItems);
    tag->putBoolean(L"CharcoalUsed", mCharcoalUsed);
    if (mName != NULL) mName->saveToTag(tag);
    return tag;
}

bool FurnaceBlockEntity::isLit()
{
    return mLitTime > 0;
}

bool FurnaceBlockEntity::isLit(boost::shared_ptr<Container> container)
{
    return container != NULL && container->getContainerData(0) > 0;
}

bool FurnaceBlockEntity::canBurn()
{
    if (mItems[0]->isEmpty()) return false;
    ItemInstance* result = FurnaceRecipes::getInstance()->getResult(mItems[0]);
    if (result->isEmpty()) return false;
    if (mItems[2]->isEmpty()) return true;
    if (!mItems[2]->sameItem_not_shared(result)) return false;
    return mItems[2]->getCount() < getMaxStackSize() && mItems[2]->getCount() < mItems[2]->getMaxStackSize();
}

void FurnaceBlockEntity::burn()
{
    if (!canBurn()) return;
    ItemInstance* result = FurnaceRecipes::getInstance()->getResult(mItems[0]);
    if (!mItems[2]->isEmpty())
    {
        if (mItems[2]->getItem() == result->getItem()) mItems[2]->grow(1);
    }
    else
    {
        mItems[2] = result->copy();
    }
    mItems[0]->shrink(1);
    if (mItems[0]->isEmpty()) mItems[0] = not_null_ptr<ItemInstance>();
}

void FurnaceBlockEntity::tick()
{
    bool wasLit = isLit();
    if (isLit()) --mLitTime;
    if (canBurn())
    {
        if (!isLit() && !mItems[1]->isEmpty())
        {
            mLitDuration = mLitTime = getBurnDuration(mItems[1]);
            if (mLitTime > 0) mItems[1]->shrink(1);
        }
        if (isLit())
        {
            ++mCookingProgress;
            if (mCookingProgress >= mCookingTotalTime) { mCookingProgress = 0; burn(); }
        }
    }
    else mCookingProgress = 0;
    if (wasLit != isLit()) setChanged();
}

boost::shared_ptr<BlockEntity> FurnaceBlockEntity::clone()
{
    boost::shared_ptr<FurnaceBlockEntity> result(new FurnaceBlockEntity());
    result->mItems = mItems;
    result->mLitTime = mLitTime;
    result->mLitDuration = mLitDuration;
    result->mCookingProgress = mCookingProgress;
    result->mCookingTotalTime = mCookingTotalTime;
    result->mName = mName;
    result->mCharcoalUsed = mCharcoalUsed;
    return result;
}

unsigned long long FurnaceBlockEntity::GetType()
{
    return 0x0000000001E00202ULL;
}
