#include "game/world/block/ShulkerBoxBlockEntity.hpp"

#include <algorithm>

#include "game/core/CensoredString.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/core/DataWalker.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/ShulkerBoxMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"

ShulkerBoxBlockEntity::ShulkerBoxBlockEntity(const DyeColor* color)
    : RandomizableLockableContainerBlockEntity()
    , WorldlyContainer()
    , InteractionObject()
    , mCleared(false)
    , mOpenCount(0)
    , mAnimationStatus(0)
    , mProgress(0.0f)
    , mProgressOld(0.0f)
    , mColor(color)
    , mDestroyedByCreativePlayer(false)
{
    _init(color);
}

ShulkerBoxBlockEntity::ShulkerBoxBlockEntity()
    : RandomizableLockableContainerBlockEntity()
    , WorldlyContainer()
    , InteractionObject()
    , mCleared(false)
    , mOpenCount(0)
    , mAnimationStatus(0)
    , mProgress(0.0f)
    , mProgressOld(0.0f)
    , mColor(NULL)
    , mDestroyedByCreativePlayer(false)
{
    _init(NULL);
}

ShulkerBoxBlockEntity::~ShulkerBoxBlockEntity()
{
}

BlockEntity* ShulkerBoxBlockEntity::create()
{
    return new ShulkerBoxBlockEntity();
}

void ShulkerBoxBlockEntity::_init(const DyeColor* color)
{
    not_null_ptr<ItemInstance> empty;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(27, empty);
    mColor = color;
    mCleared = false;
    mOpenCount = 0;
    mAnimationStatus = 0;
    mProgress = 0.0f;
    mProgressOld = 0.0f;
    mDestroyedByCreativePlayer = false;
}

std::wstring ShulkerBoxBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring(L"container.shulkerBox");
}

std::wstring ShulkerBoxBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring(L"container.shulkerBox");
}

unsigned int ShulkerBoxBlockEntity::getContainerSize()
{
    return mItems.size();
}

bool ShulkerBoxBlockEntity::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        if (mItems[i] && !mItems[i]->isEmpty())
        {
            return false;
        }
    }
    return true;
}

int ShulkerBoxBlockEntity::getMaxStackSize()
{
    return 64;
}

void ShulkerBoxBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

void ShulkerBoxBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
    ++mOpenCount;
    mCleared = false;
    mAnimationStatus = 1;
}

void ShulkerBoxBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
    if (mOpenCount > 0)
    {
        --mOpenCount;
    }
    if (mOpenCount == 0)
    {
        mAnimationStatus = 3;
    }
}

void ShulkerBoxBlockEntity::clearContent()
{
    RandomizableLockableContainerBlockEntity::clearContent();
    mCleared = true;
    mOpenCount = 0;
    mAnimationStatus = 0;
}

AbstractContainerMenu* ShulkerBoxBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory,
                                                          boost::shared_ptr<Player> player)
{
    boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(shared_from_this());
    return new ShulkerBoxMenu(container, container, player);
}

int ShulkerBoxBlockEntity::getTypeId(bool large)
{
    return 21;
}

arrayWithLength<int> ShulkerBoxBlockEntity::getSlotsForFace(const Direction* face)
{
    arrayWithLength<int> slots;
    return slots;
}

bool ShulkerBoxBlockEntity::canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> item,
                                                    const Direction* face)
{
    return canPlaceItem(slot, item);
}

bool ShulkerBoxBlockEntity::canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> item,
                                                   const Direction* face)
{
    return true;
}

int ShulkerBoxBlockEntity::getAnimationStatus()
{
    return mAnimationStatus;
}

void ShulkerBoxBlockEntity::setDestroyedByCreativePlayer(bool value)
{
    mDestroyedByCreativePlayer = value;
}

bool ShulkerBoxBlockEntity::isCleared()
{
    return mCleared;
}

bool ShulkerBoxBlockEntity::isDestroyedByCreativePlayer()
{
    return mDestroyedByCreativePlayer;
}

bool ShulkerBoxBlockEntity::shouldDrop()
{
    return !mDestroyedByCreativePlayer || !hasLevel() || onlyOpCanSetNbt() || mItems.size() == 0;
}

CompoundTag* ShulkerBoxBlockEntity::saveToTag(CompoundTag* tag)
{
    ContainerHelper::saveAllItems(tag, mItems);
    if (mColor != NULL)
    {
        tag->putInt(L"Color", mColor->getItemData());
    }
    tag->putByte(L"AnimationStatus", static_cast<unsigned char>(mAnimationStatus));
    tag->putFloat(L"Progress", mProgress);
    tag->putFloat(L"ProgressOld", mProgressOld);
    return tag;
}

float ShulkerBoxBlockEntity::getProgress(float partialTick)
{
    return mProgressOld + (mProgress - mProgressOld) * partialTick;
}

AABB* ShulkerBoxBlockEntity::getBoundingBox(const Direction* direction)
{
    float progress = getProgress(1.0f);
    double x = direction == NULL ? 0.0 : direction->getAxisDirection()->getStep() == 0 ? 0.0 : progress * direction->getAxisDirection()->getStep();
    return AABB::newTemp(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)->expand(x, x, x);
}

AABB* ShulkerBoxBlockEntity::getBoundingBox(const BlockState* state)
{
    return getBoundingBox(Direction::UP);
}

AABB* ShulkerBoxBlockEntity::getTopBoundingBox(const Direction* direction)
{
    return getBoundingBox(direction)->contract(0.0, 0.0, 0.0);
}

void ShulkerBoxBlockEntity::moveCollidedEntities()
{
}

void ShulkerBoxBlockEntity::updateAnimation()
{
    mProgressOld = mProgress;
    if (mAnimationStatus == 0)
    {
        mProgress = 0.0f;
    }
    else if (mAnimationStatus == 1)
    {
        mProgress += 0.1f;
        if (mProgress >= 1.0f)
        {
            mProgress = 1.0f;
            mAnimationStatus = 2;
            moveCollidedEntities();
        }
    }
    else if (mAnimationStatus == 3)
    {
        mProgress -= 0.1f;
        if (mProgress <= 0.0f)
        {
            mProgress = 0.0f;
            mAnimationStatus = 0;
        }
    }
}

void ShulkerBoxBlockEntity::tick()
{
    updateAnimation();
    if (mAnimationStatus == 1 || mAnimationStatus == 3)
    {
        moveCollidedEntities();
    }
}

bool ShulkerBoxBlockEntity::triggerEvent(int id, int param)
{
    if (id == 1)
    {
        mOpenCount = param;
        mAnimationStatus = param == 0 ? 3 : 1;
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void ShulkerBoxBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(static_cast<DataFixTypes::EDataFixTypes>(3),
                     (DataWalker*)new ItemListDataWalker(0x0120020fULL, L"Items"));
}

void ShulkerBoxBlockEntity::loadFromTag(CompoundTag* tag)
{
    ContainerHelper::loadAllItems(tag, mItems);
    if (tag->contains(L"Color", 3))
    {
        mColor = DyeColor::byItemData(tag->getInt(L"Color"));
    }
    if (tag->contains(L"AnimationStatus", 1))
    {
        mAnimationStatus = tag->getByte(L"AnimationStatus");
    }
    mProgress = tag->getFloat(L"Progress");
    mProgressOld = tag->getFloat(L"ProgressOld");
}

void ShulkerBoxBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    loadFromTag(tag);
}

void ShulkerBoxBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    saveToTag(tag);
}

NonNullList<not_null_ptr<ItemInstance> > ShulkerBoxBlockEntity::getItems()
{
    return mItems;
}

void ShulkerBoxBlockEntity::setItems(NonNullList<not_null_ptr<ItemInstance> > items)
{
    mItems = items;
}

const DyeColor* ShulkerBoxBlockEntity::getColor()
{
    return mColor;
}

boost::shared_ptr<Packet> ShulkerBoxBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>();
}

boost::shared_ptr<BlockEntity> ShulkerBoxBlockEntity::clone()
{
    boost::shared_ptr<ShulkerBoxBlockEntity> result(new ShulkerBoxBlockEntity(mColor));
    result->mItems = mItems;
    result->mOpenCount = mOpenCount;
    result->mCleared = mCleared;
    result->mAnimationStatus = mAnimationStatus;
    result->mProgress = mProgress;
    result->mProgressOld = mProgressOld;
    result->mDestroyedByCreativePlayer = mDestroyedByCreativePlayer;
    return result;
}

unsigned long long ShulkerBoxBlockEntity::GetType()
{
    return 0x0120020fULL;
}

void ShulkerBoxBlockEntity::operator=(const ShulkerBoxBlockEntity& other)
{
    RandomizableLockableContainerBlockEntity::operator=(other);
    mOpenCount = other.mOpenCount;
    mCleared = other.mCleared;
    mAnimationStatus = other.mAnimationStatus;
    mProgress = other.mProgress;
    mProgressOld = other.mProgressOld;
    mColor = other.mColor;
    mDestroyedByCreativePlayer = other.mDestroyedByCreativePlayer;
}
