#include "game/entity/mob/ItemEntity.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Material.hpp"
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
    mEntityData->set(DATA_ITEM, item, true);
}

void ItemEntity::setDefaultPickUpDelay()
{
    mPickupDelay = 10;
}

void ItemEntity::addDataWalkers(DataFixerUpper* dataFixerUpper)
{
    dataFixerUpper->addWalker(DataFixTypes::eDataFixTypes_Entity,
                              new ItemDataWalker(0x0800000aULL, L"Item"));
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
    mEntityData->createDateItem(DATA_ITEM, ItemInstance::EMPTY);
}

not_null_ptr<ItemInstance> ItemEntity::getItem()
{
    return mEntityData->get(DATA_ITEM);
}

void ItemEntity::setUnderwaterMovement()
{
    mXd *= 0.99;
    mYd = 0.06;
    mZd *= 0.99;
}

bool ItemEntity::merge(boost::shared_ptr<ItemEntity> entity)
{
    if (entity.get() == this)
        return false;
    if (!entity->isAlive() || !isAlive())
        return false;
    if (mPickupDelay == 32767 || entity->mPickupDelay == 32767)
        return false;
    not_null_ptr<ItemInstance> thisItem = getItem();
    not_null_ptr<ItemInstance> otherItem = entity->getItem();
    if (!otherItem->sameItem(thisItem))
        return false;
    if (otherItem->getCount() < thisItem->getCount())
        return entity->merge(boost::dynamic_pointer_cast<ItemEntity>(shared_from_this()));
    if (otherItem->getCount() + thisItem->getCount() > otherItem->getMaxStackSize())
        return false;
    otherItem->grow(thisItem->getCount());
    int mergedPickupDelay = mPickupDelay;
    if (entity->mPickupDelay > mergedPickupDelay)
        mergedPickupDelay = entity->mPickupDelay;
    int mergedAge = mAge;
    if (entity->mAge < mergedAge)
        mergedAge = entity->mAge;
    entity->mPickupDelay = mergedPickupDelay;
    entity->mAge = mergedAge;
    entity->setItem(otherItem);
    remove();
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
    Entity::tick();
    if (mPickupDelay > 0 && mPickupDelay != 32767)
        --mPickupDelay;

    mXo = mX;
    mYo = mY;
    mZo = mZ;

    double oldXd = mXd;
    double oldYd = mYd;
    double oldZd = mZd;

    if (isUnderLiquid(Material::WATER))
    {
        setNoGravity(true);
        setUnderwaterMovement();
    }
    else
    {
        setNoGravity(false);
        if (mAge % 25 == 0)
            mergeWithNeighbours();
        if (!isNoGravity())
            mYd -= 0.04;
    }
    move(eMoverType_Self, mXd, mYd, mZd, true);
    mXd *= 0.98;
    mYd *= 0.98;
    mZd *= 0.98;
    updateInWaterState();
    if (!mLevel->mIsClientSide &&
        (mXd - oldXd) * (mXd - oldXd) + (mYd - oldYd) * (mYd - oldYd) + (mZd - oldZd) * (mZd - oldZd) > 0.01)
        mHasImpulse = true;
    ++mAge;
    if (!mLevel->mIsClientSide && mAge > 5999)
        remove();
}

void ItemEntity::setShortLifeTime()
{
    mAge = 4800;
}

bool ItemEntity::updateInWaterState()
{
    bool inWater = mLevel->checkAndHandleWater(mBoundingBox, Material::WATER, shared_from_this());
    if (!inWater)
    {
        mIsInWater = false;
    }
    else
    {
        if (!mIsInWater && !mFirstTick)
            doWaterSplashEffect();
        mIsInWater = true;
    }
    return mIsInWater;
}

void ItemEntity::burn(int damage)
{
    hurt(NULL, static_cast<float>(damage));
}

bool ItemEntity::hurt(const DamageSource* source, float damage)
{
    if (mLevel->mIsClientSide || isInvulnerableTo(source))
        return false;
    bool immune = !getItem()->isEmpty() && getItem()->getItem() == Items::netherStar && source->isExplosion();
    if (!immune)
    {
        markHurt();
        mHealth -= static_cast<int>(damage);
        if (mHealth <= 0)
            remove();
    }
    return false;
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
    return L"";
}

boost::shared_ptr<Entity> ItemEntity::changeDimension(int dimension)
{
    boost::shared_ptr<Entity> newEntity = Entity::changeDimension(dimension);
    if (!mLevel->mIsClientSide && newEntity && newEntity->GetType() == GetType())
        boost::static_pointer_cast<ItemEntity>(newEntity)->mergeWithNeighbours();
    return newEntity;
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
