#include "game/entity/mob/ItemEntity.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

#include <typeinfo>
#include <vector>

const EntityDataAccessor<not_null_ptr<ItemInstance> >* ItemEntity::DATA_ITEM = NULL;

ItemEntity::ItemEntity(Level* level, double x, double y, double z, not_null_ptr<ItemInstance> item)
    : Entity(level, true), mAge(0), mPickupDelay(0), mHealth(5), mThrower(), mOwner(), mBobOffs(0.0f), mItem()
{
    _init(level, x, y, z);
    setItem(item);
}

ItemEntity::ItemEntity(Level* level, double x, double y, double z)
    : Entity(level, true), mAge(0), mPickupDelay(0), mHealth(5), mThrower(), mOwner(), mBobOffs(0.0f), mItem()
{
    _init(level, x, y, z);
}

ItemEntity::ItemEntity(Level* level)
    : Entity(level, true), mAge(0), mPickupDelay(0), mHealth(5), mThrower(), mOwner(), mBobOffs(0.0f), mItem()
{
    _init();
    setItem(ItemInstance::EMPTY);
}

ItemEntity::~ItemEntity()
{
}

ItemEntity* ItemEntity::create(Level* level)
{
    return new ItemEntity(level);
}

void ItemEntity::_init()
{
    fjDerivedCtorCalls();
    mAge = 0;
    mPickupDelay = 0;
    mHealth = 5;
    double bob = Math::random() * 3.1415927410125732;
    mBobOffs = (float)(bob + bob);
    setSize(0.25f, 0.25f);
    mCategories |= 0x400;
}

void ItemEntity::_init(Level*, double x, double y, double z)
{
    _init();
    setPos(x, y, z);
    mYRot = (float)(Math::random() * 360.0);
    mXd = (double)(float)(Math::random() * 0.20000000298023224 - 0.10000000149011612);
    mYd = 0.20000000298023224;
    mZd = (double)(float)(Math::random() * 0.20000000298023224 - 0.10000000149011612);
}

void ItemEntity::setItem(not_null_ptr<ItemInstance> item)
{
    if (mEntityData != NULL && DATA_ITEM != NULL)
        mEntityData->set(DATA_ITEM, item, true);
    mItem = item;
    mHealth = 5;
}

void ItemEntity::setDefaultPickUpDelay()
{
    mPickupDelay = 10;
}

void ItemEntity::addDataWalkers(DataFixerUpper* dataFixerUpper)
{
    ItemInstance::addDataWalkers(dataFixerUpper);
}

void ItemEntity::staticCtor()
{
    DATA_ITEM = SynchedEntityData::defineId<not_null_ptr<ItemInstance> >(
        0x000000000800000AULL, 0x0000000008000000ULL,
        EntityDataSerializers::ITEM_INSTANCE);
}

bool ItemEntity::makeStepSound()
{
    return false;
}

void ItemEntity::defineSynchedData()
{
    Entity::defineSynchedData();
    if (mEntityData != NULL && DATA_ITEM != NULL)
        mEntityData->createDateItem(DATA_ITEM, ItemInstance::EMPTY);
}

not_null_ptr<ItemInstance> ItemEntity::getItem()
{
    if (mEntityData != NULL && DATA_ITEM != NULL)
        return mEntityData->get(DATA_ITEM);
    return mItem;
}

void ItemEntity::setUnderwaterMovement()
{
    mXd *= 0.99;
    mYd = 0.06;
    mZd *= 0.99;
}

bool ItemEntity::merge(boost::shared_ptr<ItemEntity> entity)
{
    if (!entity || entity.get() == this || entity->getItem()->isEmpty())
        return false;
    if (getItem()->getItem() != entity->getItem()->getItem() || !getItem()->sameItem(entity->getItem()))
        return false;
    getItem()->grow(entity->getItem()->getCount());
    entity->remove();
    return true;
}

void ItemEntity::mergeWithNeighbours()
{
    if (mLevel == NULL || mBoundingBox == NULL)
        return;
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(ItemEntity), mBoundingBox->grow(0.5, 0.5, 0.5));
    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<ItemEntity> item = boost::dynamic_pointer_cast<ItemEntity>(*it);
        if (item)
            merge(item);
    }
    delete entities;
}

void ItemEntity::tick()
{
    if (getItem()->isEmpty())
    {
        remove();
        return;
    }
    if (mAge != 0x7fffffff)
        ++mAge;
    if (mAge >= 6000)
        remove();
    if (isInWater())
        setUnderwaterMovement();
    else
        mYd -= 0.04;
    move(eMoverType_Self, mXd, mYd, mZd, true);
    mergeWithNeighbours();
}

void ItemEntity::setShortLifeTime()
{
    mAge = 4800;
}

bool ItemEntity::updateInWaterState()
{
    return Entity::updateInWaterState();
}

void ItemEntity::burn(int damage)
{
    hurt(NULL, static_cast<float>(damage));
}

bool ItemEntity::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;
    mHealth -= static_cast<int>(damage);
    markHurt();
    if (mHealth <= 0)
        remove();
    return true;
}

std::wstring ItemEntity::getThrower()
{
    return mThrower;
}

std::wstring ItemEntity::getOwner()
{
    return mOwner;
}

void ItemEntity::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putShort(L"Health", static_cast<short>(mHealth));
    tag->putShort(L"Age", static_cast<short>(mAge));
    tag->putShort(L"PickupDelay", static_cast<short>(mPickupDelay));
    if (!mThrower.empty())
        tag->putString(L"Thrower", mThrower);
    if (!mOwner.empty())
        tag->putString(L"Owner", mOwner);
    if (!mItem->isEmpty())
        tag->put(L"Item", mItem->save(new CompoundTag()));
}

void ItemEntity::readAdditionalSaveData(CompoundTag* tag)
{
    mHealth = tag->getShort(L"Health");
    mAge = tag->getShort(L"Age");
    if (tag->contains(L"PickupDelay"))
        mPickupDelay = tag->getShort(L"PickupDelay");
    if (tag->contains(L"Owner"))
        mOwner = tag->getString(L"Owner");
    if (tag->contains(L"Thrower"))
        mThrower = tag->getString(L"Thrower");
    if (tag->contains(L"Item"))
        setItem(not_null_ptr<ItemInstance>(new ItemInstance(tag->getCompound(L"Item"))));
    if (mItem->isEmpty())
        remove();
}

void ItemEntity::playerTouch(boost::shared_ptr<Player> player)
{
    if (mLevel->mIsClientSide)
        return;

    not_null_ptr<ItemInstance> item = getItem();
    if (item->isEmpty())
    {
        remove();
        return;
    }

    Item* itemType = item->getItem();
    int count = item->getCount();
    if (mPickupDelay != 0)
        return;

    if (!mOwner.empty() && mAge < 5800 && player->getName() != mOwner)
        return;

    bool replacedExisting = false;
    if (!player->AutoEquip(item, item, replacedExisting) && !player->mInventory->add(item))
        return;

    if (itemType == Items::diamond)
    {
        player->awardStat(GenericStats::diamonds(), GenericStats::param_diamonds());
        if (!mThrower.empty())
        {
            boost::shared_ptr<Player> thrower = mLevel->getPlayerByName(mThrower);
            if (thrower && thrower != player)
                thrower->awardStat(GenericStats::diamondsToYou(), GenericStats::param_diamondsToYou());
        }
    }
    if (itemType == Items::blazeRod)
        player->awardStat(GenericStats::blazeRod(), GenericStats::param_blazeRod());

    player->take(shared_from_this(), count);
    if (item->isEmpty())
    {
        remove();
        item->setCount(count);
    }
}

std::wstring ItemEntity::getName()
{
    if (hasCustomName())
        return getCustomName();
    return std::wstring();
}

boost::shared_ptr<Entity> ItemEntity::changeDimension(int dimension)
{
    mergeWithNeighbours();
    return Entity::changeDimension(dimension);
}

bool ItemEntity::isAttackable()
{
    return false;
}

void ItemEntity::setOwner(const std::wstring& owner)
{
    mOwner = owner;
}

void ItemEntity::setThrower(const std::wstring& thrower)
{
    mThrower = thrower;
}

void ItemEntity::setNoPickUpDelay()
{
    mPickupDelay = 0;
}

void ItemEntity::setNeverPickUp()
{
    mPickupDelay = 32767;
}

void ItemEntity::setPickUpDelay(int delay)
{
    mPickupDelay = delay;
}

bool ItemEntity::hasPickUpDelay()
{
    return mPickupDelay > 0;
}

void ItemEntity::setExtendedLifetime()
{
    mAge = -6000;
}

void ItemEntity::makeFakeItem()
{
    setNeverPickUp();
    mAge = 5999;
}

unsigned long long ItemEntity::GetType()
{
    return 0x0800000aULL;
}

int ItemEntity::getAge()
{
    return mAge;
}
