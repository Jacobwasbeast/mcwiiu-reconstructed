#include "game/world/block/BeaconBlockEntity.hpp"

#include <algorithm>
#include <typeinfo>

#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Inventory.hpp"
#include "game/world/Level.hpp"
#include "game/util/AABB.hpp"
#include "game/ui/BeaconMenu.hpp"

namespace
{
MobEffect* sBeaconEffects[12] = {0};
MobEffect* sBeaconEffectOrder[6] = {0};
int sBeaconEffectCount = 0;
}

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
    : LockableContainerBlockEntity(), WorldlyContainer(),
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
    (void)player;
    boost::shared_ptr<Container> beacon = boost::dynamic_pointer_cast<Container>(shared_from_this());
    boost::shared_ptr<Container> playerContainer = boost::static_pointer_cast<Container>(inventory);
    return new BeaconMenu(playerContainer, beacon);
}

int BeaconBlockEntity::getTypeId(bool large)
{
    return 7;
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
    if (mLevel == NULL || mLevels <= 0 || mPrimaryPower == NULL) return;
    const int radius = mLevels * 10 + 10;
    AABB searchArea(static_cast<double>(mPos.x - radius), static_cast<double>(mPos.y - radius),
                    static_cast<double>(mPos.z - radius), static_cast<double>(mPos.x + radius + 1),
                    static_cast<double>(mPos.y + radius + 1), static_cast<double>(mPos.z + radius + 1));
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Player), &searchArea);
    if (entities == NULL) return;
    const int duration = radius * 20;
    const int amplifier = mLevels >= 4 ? 1 : 0;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(*it);
        if (player != NULL) player->addEffect(new MobEffectInstance(mPrimaryPower, duration, amplifier), boost::shared_ptr<Entity>());
    }
    if (mLevels >= 4 && mSecondaryPower != NULL && mSecondaryPower != mPrimaryPower)
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(*it);
            if (player != NULL) player->addEffect(new MobEffectInstance(mSecondaryPower, duration, 0), boost::shared_ptr<Entity>());
        }
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
    sBeaconEffects[level * 3 + slot] = effect;
    sBeaconEffectOrder[sBeaconEffectCount++] = effect;
}

void BeaconBlockEntity::staticCtor()
{
    for (int i = 0; i < 12; ++i) sBeaconEffects[i] = NULL;
    for (int i = 0; i < 6; ++i) sBeaconEffectOrder[i] = NULL;
    sBeaconEffectCount = 0;
    AddEffect(0, 0, MobEffects::MOVEMENT_SPEED);
    AddEffect(0, 1, MobEffects::DIG_SPEED);
    AddEffect(1, 0, MobEffects::DAMAGE_RESISTANCE);
    AddEffect(1, 1, MobEffects::JUMP);
    AddEffect(2, 0, MobEffects::DAMAGE_BOOST);
    AddEffect(3, 0, MobEffects::REGENERATION);
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

CompoundTag* BeaconBlockEntity::save(CompoundTag* tag)
{
    LockableContainerBlockEntity::save(tag);
    tag->putInt(L"Levels", mLevels);
    tag->putInt(L"BeamDirection", mBeamDirection);
    tag->putInt(L"BeamLength", mBeamLength);
    mName->saveToTag(tag);
    return tag;
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
    return 0x0000000001E00204ULL;
}
