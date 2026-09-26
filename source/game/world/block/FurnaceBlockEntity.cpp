#include "game/world/block/FurnaceBlockEntity.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/FurnaceFuelSlot.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Mth.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/FurnaceRecipes.hpp"
#include "game/item/BoatItem.hpp"
#include "game/item/DiggerItem.hpp"
#include "game/item/DoorItem.hpp"
#include "game/item/HoeItem.hpp"
#include "game/item/SwordItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/ui/FurnaceMenu.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FurnaceBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

FurnaceBlockEntity::FurnaceBlockEntity()
    : LockableContainerBlockEntity(), WorldlyContainer(),
      mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(3, ItemInstance::EMPTY)),
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
    if (mLevel->getBlockEntity(mPos) != shared_from_this())
    {
        return false;
    }
    return player->distanceAsIfHitscanned(mPos, *player->mBoundingBox) <= 30.25;
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
    if (item->isEmpty()) return 0;
    Item* itemType = item->getItem();
    if (itemType == Item::byBlock(Blocks::wooden_slab)) return 150;
    if (itemType == Item::byBlock(Blocks::wool)) return 100;
    if (itemType == Item::byBlock(Blocks::carpet)) return 67;
    if (itemType != Item::byBlock(Blocks::ladder))
    {
        if (itemType == Item::byBlock(Blocks::wooden_button)) return 100;
        if (itemType == Item::byBlock(Blocks::acacia_button)) return 100;
        if (itemType == Item::byBlock(Blocks::birch_button)) return 100;
        if (itemType == Item::byBlock(Blocks::dark_oak_button)) return 100;
        if (itemType == Item::byBlock(Blocks::jungle_button)) return 100;
        if (itemType == Item::byBlock(Blocks::spruce_button)) return 100;
        Block* block = Block::byItem(itemType);
        const BlockState* state = block->defaultBlockState();
        Material* material = state->getMaterial();
        if (material != Material::WOOD)
        {
            if (itemType == Item::byBlock(Blocks::coal_block)) return 16000;
            DiggerItem* digger = dynamic_cast<DiggerItem*>(itemType);
            if (digger != NULL && digger->getTier() == Item::Tier::WOOD) return 200;
            SwordItem* sword = dynamic_cast<SwordItem*>(itemType);
            if (sword != NULL && sword->getTier() == Item::Tier::WOOD) return 200;
            HoeItem* hoe = dynamic_cast<HoeItem*>(itemType);
            if (hoe != NULL && hoe->getTier() == Item::Tier::WOOD) return 200;
            if (itemType == Items::stick) return 100;
            if (itemType == Items::bow) return 300;
            if (itemType == Items::fishingRod) return 300;
            if (itemType == Items::sign) return 200;
            if (itemType == Items::coal) return 1600;
            if (itemType == Items::lavaBucket) return 20000;
            if (itemType == Item::byBlock(Blocks::sapling)) return 100;
            if (itemType == Items::bowl) return 100;
            if (itemType == Items::blazeRod) return 2400;
            if (itemType == Item::byBlock(Blocks::dried_kelp_block)) return 4000;
            if (itemType == NULL) return 0;
            if (dynamic_cast<DoorItem*>(itemType) != NULL && itemType != Items::ironDoor) return 200;
            if (dynamic_cast<BoatItem*>(itemType) != NULL) return 400;
            return 0;
        }
    }
    return 300;
}

bool FurnaceBlockEntity::isFuel(not_null_ptr<ItemInstance> item)
{
    return getBurnDuration(item) > 0;
}

bool FurnaceBlockEntity::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    if (slot == 2) return false;
    if (slot != 1) return true;
    return isFuel(item) || (FurnaceFuelSlot::isBucket(item) && mItems[1]->getItem() != Items::bucket);
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
    for (unsigned int i = 0; i < mItems.size(); ++i) mItems[i] = ItemInstance::EMPTY;
}

AbstractContainerMenu* FurnaceBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player)
{
    return new FurnaceMenu(inventory, boost::static_pointer_cast<FurnaceBlockEntity>(shared_from_this()));
}

int FurnaceBlockEntity::getTypeId(bool large)
{
    return ClientboundContainerOpenPacket::FURNACE;
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
    if (face == Direction::DOWN && slot == 1)
    {
        Item* itemType = item->getItem();
        if (itemType != Items::waterBucket && itemType != Items::bucket)
            return false;
    }
    return true;
}

void FurnaceBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal)
{
    if (mName != NULL) mName->set(name, uuid, chatCensored, uuidLocal);
}

void FurnaceBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemListDataWalker(BlockEntityType::FURNACE, L"Items"));
}

void FurnaceBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
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
    if (mItems[2]->getCount() < getMaxStackSize() && mItems[2]->getCount() < mItems[2]->getMaxStackSize()) return true;
    return mItems[2]->getCount() < result->getMaxStackSize();
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
    if (mItems[0]->getItem() == Item::byBlock(Blocks::sponge) && mItems[0]->getAuxValue() == 1
        && !mItems[1]->isEmpty() && mItems[1]->getItem() == Items::bucket)
    {
        mItems[1] = not_null_ptr<ItemInstance>(new ItemInstance(Items::waterBucket));
    }
    mItems[0]->shrink(1);
}

void FurnaceBlockEntity::tick()
{
    bool wasLit = isLit();
    bool changed = false;
    if (wasLit) --mLitTime;
    if (mLevel == NULL) return;
    if (mLevel->mIsClientSide) return;

    if (!isLit() && !mItems[1]->isEmpty() && !mItems[0]->isEmpty() && canBurn())
    {
        mLitDuration = mLitTime = getBurnDuration(mItems[1]);
        if (isLit())
        {
            changed = true;
            Item* fuelItem = mItems[1]->getItem();
            if (fuelItem == Items::coal)
            {
                if (mItems[1]->getAuxValue() == 1) mCharcoalUsed = true;
            }
            else if (fuelItem == Items::lavaBucket)
            {
                for (std::set<boost::shared_ptr<Player> >::iterator it = mOpeners.begin(); it != mOpeners.end(); ++it)
                {
                    (*it)->awardStat(GenericStats::superFuel(), GenericStats::param_superFuel());
                }
            }
            else if (fuelItem == Item::byBlock(Blocks::dried_kelp_block))
            {
                for (std::set<boost::shared_ptr<Player> >::iterator it = mOpeners.begin(); it != mOpeners.end(); ++it)
                {
                    (*it)->awardStat(GenericStats::alternativeFuel(), GenericStats::param_alternativeFuel());
                }
            }
            mItems[1]->shrink(1);
            if (mItems[1]->isEmpty())
            {
                Item* remainder = fuelItem->getCraftingRemainingItem();
                mItems[1] = (remainder != NULL) ? not_null_ptr<ItemInstance>(new ItemInstance(remainder)) : ItemInstance::EMPTY;
            }
            mOpenersThisTick = mOpeners;
        }
    }

    if (isLit())
    {
        if (canBurn())
        {
            if (++mCookingProgress == mCookingTotalTime)
            {
                mCookingProgress = 0;
                mCookingTotalTime = getTotalCookTime(mItems[0]);
                burn();
                changed = true;
            }
        }
        else
        {
            mCookingProgress = 0;
        }
    }
    else if (mCookingProgress > 0)
    {
        mCookingProgress = Mth::clamp(mCookingProgress - 2, 0, mCookingTotalTime);
    }

    if (wasLit != isLit())
    {
        changed = true;
        FurnaceBlock::setLit(isLit(), mLevel, mPos);
    }
    if (changed) setChanged();
}

boost::shared_ptr<BlockEntity> FurnaceBlockEntity::clone()
{
    boost::shared_ptr<FurnaceBlockEntity> result(new FurnaceBlockEntity());
    BlockEntity::clone(result);
    result->mLitTime = mLitTime;
    result->mLitDuration = mLitDuration;
    result->mCookingProgress = mCookingProgress;
    result->mCookingTotalTime = mCookingTotalTime;
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        result->mItems[i] = mItems[i]->copy();
    }
    result->mName->set(mName->raw(), mName->getUUID(), mName->isChatCensored(), mName->isUUIDLocal());
    return result;
}

unsigned long long FurnaceBlockEntity::GetType()
{
    return BlockEntityType::FURNACE;
}
