#include "game/entity/mob/AgableMob.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/client/render/ParticleType.hpp"

const EntityDataAccessor<bool>* AgableMob::DATA_BABY_ID = NULL;
const EntityDataAccessor<bool>* AgableMob::DATA_BABY_ID_2 = NULL;

AgableMob::AgableMob(Level* level)
    : PathfinderMob(level), mAge(0), mForcedAge(0), mForcedAgeTimer(0),
      mBaseWidth(-1.0f), mBaseHeight(0.0f), mUninitialized(0)
{
}

AgableMob::~AgableMob()
{
}

void AgableMob::staticCtor()
{
    DATA_BABY_ID = SynchedEntityData::defineId(0x000000000D600000ULL, 0x000000000D400000ULL,
                                               EntityDataSerializers::BOOLEAN);
    DATA_BABY_ID_2 = SynchedEntityData::defineId(0x000000000D600000ULL, 0x000000000D400000ULL,
                                                 EntityDataSerializers::BOOLEAN);
}

bool AgableMob::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = ((LivingEntity*)player.get())->getItemInHand(hand);
    if (Item::getId(itemInstance->getItem()) != 383 || isBaby())
    {
        return false;
    }

    unsigned long long type = SpawnEggItem::fjGetType(itemInstance);
    if (type != GetType())
    {
        return false;
    }

    ResourceLocation entityType = SpawnEggItem::getNamedIdFrom(itemInstance);
    int error = -1;
    if (!SpawnEggItem::canSpawn(entityType, mLevel, &error))
    {
        SpawnEggItem::DisplaySpawnError(player, error);
        return true;
    }
    if (!mLevel->canCreateMore(GetType(), 0))
    {
        return true;
    }

    boost::shared_ptr<Mob> mob = SpawnEggItem::getMob(mLevel, entityType, mX, mY, mZ, &error, std::wstring(L""));
    if (!mob)
    {
        SpawnEggItem::DisplaySpawnError(player, error);
        return true;
    }

    mob->moveTo(mX, mY, mZ, mYRot, 0.0f);
    boost::shared_ptr<AgableMob> agable = boost::dynamic_pointer_cast<AgableMob>(mob);
    if (agable)
    {
        agable->setAge(-24000);
    }
    mLevel->addEntity(mob);
    if (itemInstance->hasCustomHoverName())
    {
        mob->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                           itemInstance->wasCreatedByRestrictedPlayer());
    }
    if (!((LivingEntity*)player.get())->isCreative())
    {
        itemInstance->shrink(1);
    }
    return true;
}

bool AgableMob::holdingSpawnEggOfClass(const not_null_ptr<ItemInstance>& itemInstance, unsigned long long type)
{
    if (Item::getId(itemInstance->getItem()) != 383)
    {
        return false;
    }
    return SpawnEggItem::fjGetType(itemInstance) == type;
}

void AgableMob::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_BABY_ID, false);
    mEntityData->createDateItem(DATA_BABY_ID_2, false);
}

int AgableMob::getAge()
{
    if (mLevel == NULL || mLevel->mIsClientSide)
    {
        if (mEntityData->get(DATA_BABY_ID_2))
        {
            return 1;
        }
        if (mEntityData->get(DATA_BABY_ID))
        {
            return -1;
        }
        return 0;
    }
    return mAge;
}

void AgableMob::ageUp(int amount, bool forced)
{
    int oldAge = getAge();
    int age = oldAge + amount * 20;
    if (age > 0)
    {
        age = 0;
        if (oldAge < 0)
        {
            grewUp();
        }
    }
    setAge(age);
    if (forced)
    {
        mForcedAge += age - oldAge;
        if (mForcedAgeTimer == 0)
        {
            mForcedAgeTimer = 40;
        }
    }
    if (getAge() == 0)
    {
        setAge(mForcedAge);
    }
}

void AgableMob::ageUp(int amount)
{
    ageUp(amount, false);
}

void AgableMob::setAge(int age)
{
    if (mLevel == NULL || !mLevel->mIsClientSide)
    {
        mEntityData->set(DATA_BABY_ID, age < 0, true);
        mEntityData->set(DATA_BABY_ID_2, age > 0, true);
    }
    mAge = age;
    updateSize(isBaby());
}

void AgableMob::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(std::wstring(L"Age"), getAge());
    tag->putInt(std::wstring(L"ForcedAge"), mForcedAge);
}

void AgableMob::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    setAge(tag->getInt(std::wstring(L"Age")));
    mForcedAge = tag->getInt(std::wstring(L"ForcedAge"));
}

void AgableMob::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (accessor->getId() == DATA_BABY_ID->getId())
    {
        updateSize(isBaby());
    }
}

void AgableMob::aiStep()
{
    Mob::aiStep();
    if (!mLevel->mIsClientSide)
    {
        int age = getAge();
        if (age < 0)
        {
            setAge(age + 1);
            if (age + 1 == 0)
            {
                grewUp();
            }
        }
        else if (age > 0)
        {
            setAge(age - 1);
        }
    }
    else if (mForcedAgeTimer > 0)
    {
        if ((mForcedAgeTimer & 3) == 0)
        {
            mLevel->addParticle(PARTICLE_HAPPY_VILLAGER,
                                mX + (mRandom->nextFloat() * mBbWidth * 2.0f) - mBbWidth,
                                mY + 0.5 + mRandom->nextFloat() * mBbHeight,
                                mZ + (mRandom->nextFloat() * mBbWidth * 2.0f) - mBbWidth,
                                0.0, 0.0, 0.0, arrayWithLength<int>());
        }
        --mForcedAgeTimer;
    }
}

bool AgableMob::isBaby()
{
    return getAge() < 0;
}

unsigned int AgableMob::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (Item::getId(holder.m_pItemInstance->getItem()) == 383
        && SpawnEggItem::fjGetType(holder.m_pItemInstance) == GetType())
    {
        return 0x4C36CD96;
    }
    return Mob::GetInteractTooltip(holder);
}

void AgableMob::grewUp()
{
}

void AgableMob::internalSetSize(float scale)
{
    Entity::setSize(mBaseWidth * scale, mBaseHeight * scale);
}

void AgableMob::updateSize(bool baby)
{
    internalSetSize(baby ? 0.5f : 1.0f);
}

void AgableMob::setSize(float width, float height)
{
    float oldWidth = mBaseWidth;
    mBaseHeight = height;
    mBaseWidth = width;
    if (oldWidth <= 0.0f)
    {
        internalSetSize(1.0f);
    }
}
