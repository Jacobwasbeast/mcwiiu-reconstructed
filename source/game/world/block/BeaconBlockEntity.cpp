#include "game/world/block/BeaconBlockEntity.hpp"

#include <algorithm>

#include "game/entity/mob/Entity.hpp"

BeaconBlockEntity::BeaconBeamSection::BeaconBeamSection(arrayWithLength<float> color)
    : mRed(color.length > 0 ? color.data[0] : 0.0F),
      mGreen(color.length > 1 ? color.data[1] : 0.0F),
      mBlue(color.length > 2 ? color.data[2] : 0.0F), mHeight(1)
{
}

float* BeaconBlockEntity::BeaconBeamSection::getColor()
{
    return &mRed;
}

void BeaconBlockEntity::BeaconBeamSection::setHeight(int height)
{
    mHeight = height;
}

void BeaconBlockEntity::BeaconBeamSection::increaseHeight()
{
    ++mHeight;
}

unsigned int BeaconBlockEntity::BeaconBeamSection::getHeight()
{
    return mHeight;
}

BeaconBlockEntity::BeaconBlockEntity()
    : LockableContainerBlockEntity(), WorldlyContainer(), InteractionObject(),
      mBeamSections(), mItem(), mName(new CensoredString()), mPrimaryPower(NULL),
      mSecondaryPower(NULL), mLevels(0), mNextClientSideUpdate(0), mLastCheckY(-1),
      mClientSideScale(0.0F), mActive(false), mBeamDirection(0), mBeamLength(0)
{
}

BeaconBlockEntity::~BeaconBlockEntity()
{
}

std::wstring BeaconBlockEntity::getName()
{
    return mName != NULL ? mName->get() : std::wstring();
}

std::wstring BeaconBlockEntity::getCustomName()
{
    return mName != NULL ? mName->get() : std::wstring();
}

bool BeaconBlockEntity::hasCustomName()
{
    return mName != NULL && !mName->raw().empty();
}

unsigned int BeaconBlockEntity::getContainerSize()
{
    return 1;
}

bool BeaconBlockEntity::isEmpty()
{
    return mItem == NULL || mItem->isEmpty();
}

not_null_ptr<ItemInstance> BeaconBlockEntity::getItem(unsigned int slot)
{
    if (slot == 0 && mItem != NULL)
    {
        return mItem;
    }
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> BeaconBlockEntity::removeItem(unsigned int slot, int count)
{
    if (slot != 0 || mItem == NULL || mItem->isEmpty() || count <= 0)
    {
        return ItemInstance::EMPTY;
    }
    not_null_ptr<ItemInstance> result = mItem->split(count);
    if (mItem->isEmpty())
    {
        mItem = ItemInstance::EMPTY;
    }
    setChanged();
    return result;
}

not_null_ptr<ItemInstance> BeaconBlockEntity::removeItemNoUpdate(int slot)
{
    if (slot != 0)
    {
        return ItemInstance::EMPTY;
    }
    not_null_ptr<ItemInstance> result = mItem;
    mItem = ItemInstance::EMPTY;
    return result;
}

void BeaconBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    if (slot == 0)
    {
        mItem = item;
        if (mItem != NULL && mItem->getCount() > getMaxStackSize())
        {
            mItem->setCount(getMaxStackSize());
        }
        setChanged();
    }
}

int BeaconBlockEntity::getMaxStackSize()
{
    return 1;
}

void BeaconBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

bool BeaconBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    if (player.get() == NULL || mLevel == NULL)
    {
        return false;
    }
    Entity* entity = (Entity*)player.get();
    return entity->distanceAsIfHitscanned(mPos, *entity->mBoundingBox) <= 30.25;
}

void BeaconBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
}

void BeaconBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
}

bool BeaconBlockEntity::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    if (slot != 0 || item == NULL)
    {
        return false;
    }
    return item->getItem() != NULL;
}

int BeaconBlockEntity::getContainerData(int id)
{
    if (id == 0) return mLevels;
    if (id == 1) return mPrimaryPower == NULL ? 0 : 1;
    if (id == 2) return mSecondaryPower == NULL ? 0 : 1;
    return 0;
}

int BeaconBlockEntity::getValidEffectById(int id)
{
    return id;
}

void BeaconBlockEntity::setContainerData(int id, int value)
{
    if (id == 0) mLevels = value;
}

int BeaconBlockEntity::getContainerDataCount()
{
    return 3;
}

void BeaconBlockEntity::clearContent()
{
    mItem = ItemInstance::EMPTY;
}

AbstractContainerMenu* BeaconBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player)
{
    return NULL;
}

int BeaconBlockEntity::getTypeId(bool large)
{
    return 0;
}

InteractionObject::NameData BeaconBlockEntity::GetNetworkNameData()
{
    return InteractionObject::NameData(getName(), std::wstring(), true, false);
}

arrayWithLength<int> BeaconBlockEntity::getSlotsForFace(const Direction* face)
{
    return arrayWithLength<int>(1, true);
}

bool BeaconBlockEntity::canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face)
{
    return canPlaceItem(slot, item);
}

bool BeaconBlockEntity::canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face)
{
    return slot == 0;
}

void BeaconBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal)
{
    if (mName != NULL) mName->set(name, uuid, chatCensored, uuidLocal);
}

int BeaconBlockEntity::getBeamDirection()
{
    return mBeamDirection;
}

void BeaconBlockEntity::updateShapeForGlide()
{
    if (!mBeamSections.empty()) mBeamSections.back().setHeight(mBeamLength);
}

bool BeaconBlockEntity::NeedsRebuild()
{
    return mLastCheckY < 0;
}

void BeaconBlockEntity::updateShape()
{
    if (NeedsRebuild()) updateShapeForGlide();
}

void BeaconBlockEntity::applyEffects()
{
}

void BeaconBlockEntity::checkValidity()
{
    if (mLevel == NULL) return;
    updateShape();
    applyEffects();
}

boost::shared_ptr<BlockEntity> BeaconBlockEntity::clone()
{
    boost::shared_ptr<BeaconBlockEntity> copy(new BeaconBlockEntity());
    copy->mItem = mItem;
    copy->mName = mName;
    copy->mPrimaryPower = mPrimaryPower;
    copy->mSecondaryPower = mSecondaryPower;
    copy->mLevels = mLevels;
    copy->mBeamSections = mBeamSections;
    copy->mBeamDirection = mBeamDirection;
    copy->mBeamLength = mBeamLength;
    copy->BlockEntity::operator=(*this);
    return copy;
}

void BeaconBlockEntity::AddEffect(int level, int slot, MobEffect* effect)
{
}

void BeaconBlockEntity::staticCtor()
{
}

void BeaconBlockEntity::tick()
{
    if (mLevel != NULL) checkValidity();
}

std::vector<BeaconBlockEntity::BeaconBeamSection>* BeaconBlockEntity::getBeamSections()
{
    return &mBeamSections;
}

double BeaconBlockEntity::getAndUpdateClientSideScale()
{
    return mClientSideScale;
}

boost::shared_ptr<Packet> BeaconBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>();
}

CompoundTag* BeaconBlockEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return tag;
}

double BeaconBlockEntity::getViewDistance()
{
    return 256.0;
}

void BeaconBlockEntity::load(CompoundTag* tag)
{
    LockableContainerBlockEntity::load(tag);
    mLevels = tag->getInt(L"Levels");
    mBeamDirection = tag->getInt(L"BeamDirection");
    mBeamLength = tag->getInt(L"BeamLength");
    mName->loadFromTag(tag);
}

void BeaconBlockEntity::save(CompoundTag* tag)
{
    LockableContainerBlockEntity::save(tag);
    tag->putInt(L"Levels", mLevels);
    tag->putInt(L"BeamDirection", mBeamDirection);
    tag->putInt(L"BeamLength", mBeamLength);
    mName->saveToTag(tag);
}

bool BeaconBlockEntity::triggerEvent(int id, int param)
{
    if (id == 1)
    {
        mLevels = param;
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void BeaconBlockEntity::setBeamDirection(eBEAM_DIRECTION direction)
{
    mBeamDirection = direction;
}

void BeaconBlockEntity::setBeamLength(int length)
{
    mBeamLength = length;
}

std::wstring BeaconBlockEntity::getDisplayName()
{
    return hasCustomName() ? getCustomName() : getName();
}

BlockEntity* BeaconBlockEntity::create()
{
    return new BeaconBlockEntity();
}

unsigned long long BeaconBlockEntity::GetType()
{
    return 0x01E0020400000000ULL;
}
