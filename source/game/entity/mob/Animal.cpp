#include "game/entity/mob/Animal.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

const EntityDataAccessor<int>* Animal::DATA_IN_LOVE = NULL;
const EntityDataAccessor<int>* Animal::DATA_IN_LOVE_TIME = NULL;

Animal::Animal(Level* level) : AgableMob(level), mType(0), mLoveCause(), mLoveCausePresent(0), mInLove(0), mLoveCauseId(0), mDespawnProtected(false), mMinX(0), mMaxX(0), mMinZ(0), mMaxZ(0), mRandomSeed(1200)
{
    setDespawnProtected();
}

Animal::~Animal() {}

void Animal::staticCtor()
{
    DATA_IN_LOVE = SynchedEntityData::defineId(0, 0x000000000D600000ULL, EntityDataSerializers::INT);
    DATA_IN_LOVE_TIME = SynchedEntityData::defineId(0, 0x000000000D600000ULL, EntityDataSerializers::INT);
}

void Animal::setDespawnProtected()
{
    if (mLevel != NULL && mLevel->mIsClientSide) return;
    mMinX = static_cast<short>(mX);
    mMaxX = static_cast<short>(mX);
    mMinZ = static_cast<short>(mZ);
    mMaxZ = static_cast<short>(mZ);
    mDespawnProtected = true;
}

void Animal::defineSynchedData()
{
    AgableMob::defineSynchedData();
    if (mEntityData != NULL && DATA_IN_LOVE != NULL) mEntityData->createDateItem(DATA_IN_LOVE, 0);
    if (mEntityData != NULL && DATA_IN_LOVE_TIME != NULL) mEntityData->createDateItem(DATA_IN_LOVE_TIME, 0);
}

void Animal::setInLoveValue(int value)
{
    if ((mLevel == NULL || !mLevel->mIsClientSide) && mEntityData != NULL && DATA_IN_LOVE != NULL) mEntityData->set(DATA_IN_LOVE, value, true);
    mInLove = value;
}

void Animal::customServerAiStep()
{
    if (isBaby()) setInLoveValue(0);
    Mob::customServerAiStep();
}

int Animal::getInLoveValue()
{
    return mEntityData != NULL && DATA_IN_LOVE != NULL ? mEntityData->get(DATA_IN_LOVE) : mInLove;
}

void Animal::updateDespawnProtectedState()
{
    if ((mLevel != NULL && mLevel->mIsClientSide) || !mDespawnProtected) return;
    --mMinX;
    --mMaxX;
    --mMinZ;
    --mMaxZ;
    if (mMaxX - mMinX > 20 || mMaxZ - mMinZ > 20) mDespawnProtected = false;
}

void Animal::aiStep()
{
    AgableMob::aiStep();
    if (getInLoveValue() > 0) setInLoveValue(getInLoveValue() - 1);
    updateDespawnProtectedState();
}

bool Animal::hurt(const DamageSource* source, float damage)
{
    setInLoveValue(0);
    return LivingEntity::hurt(source, damage);
}

double Animal::getWalkTargetValue(const BlockPos& pos)
{
    return static_cast<double>(mLevel->getBrightness(pos, 0)) - 0.5;
}

void Animal::addAdditonalSaveData(CompoundTag* tag)
{
    AgableMob::addAdditonalSaveData(tag);
    tag->putInt(L"InLove", getInLoveValue());
    if (mLoveCausePresent != 0) tag->putString(L"LoveCause", mLoveCause);
}

double Animal::getRidingHeight() { return 0.14; }

void Animal::readAdditionalSaveData(CompoundTag* tag)
{
    AgableMob::readAdditionalSaveData(tag);
    setInLoveValue(tag->getInt(L"InLove"));
    if (tag->contains(L"LoveCause")) { mLoveCause = tag->getString(L"LoveCause"); mLoveCausePresent = 1; }
    else { mLoveCause.clear(); mLoveCausePresent = 0; }
    setDespawnProtected();
}

bool Animal::checkSpawnRules()
{
    BlockPos pos(mX, mY, mZ);
    return mLevel->getDaytimeRawBrightness(pos.below()) > 8 && PathfinderMob::checkSpawnRules();
}

int Animal::getAmbientSoundInterval() { return 120; }

bool Animal::removeWhenFarAway() { return !isDespawnProtected() && !isLeashed() && !hasCustomName(); }

int Animal::getExperienceReward(boost::shared_ptr<Player> player) { return mRandom->nextInt(3) + 1; }

bool Animal::isFood(const not_null_ptr<ItemInstance>& itemInstance) { return itemInstance->getItem() == Items::wheat; }

bool Animal::isBreedCoolingDown()
{
    return mEntityData != NULL && DATA_IN_LOVE_TIME != NULL && mEntityData->get(DATA_IN_LOVE_TIME) != 0;
}

void Animal::usePlayerItem(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance)
{
    itemInstance->shrink(1);
}

bool Animal::AgeUpBaby(const boost::shared_ptr<Player>& player, not_null_ptr<ItemInstance> itemInstance)
{
    if (!isBaby() || !isFood(itemInstance)) return false;
    usePlayerItem(player, itemInstance);
    ageUp(6 * 20, true);
    return true;
}

bool Animal::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    return AgableMob::mobInteract(player, hand);
}

void Animal::setInLove(boost::shared_ptr<Player> player)
{
    setInLoveValue(600);
    mLoveCausePresent = 0;
    if (mEntityData != NULL && DATA_IN_LOVE_TIME != NULL) mEntityData->set(DATA_IN_LOVE_TIME, 18, true);
}

boost::shared_ptr<Player> Animal::getLoveCause() { return boost::shared_ptr<Player>(); }
bool Animal::isInLove() { return getInLoveValue() != 0; }

void Animal::resetLove()
{
    setInLoveValue(0);
    if (mEntityData != NULL && DATA_IN_LOVE_TIME != NULL) mEntityData->set(DATA_IN_LOVE_TIME, mRandomSeed, true);
}

bool Animal::canMate(const boost::shared_ptr<Animal>& other)
{
    return other != NULL && other.get() != this && GetType() == other->GetType() && isInLove() && other->isInLove();
}

void Animal::handleEntityEvent(unsigned char event, int data) { LivingEntity::handleEntityEvent(event, data); }
unsigned int Animal::GetInteractTooltip(const EntityTooltipDataHolder& holder) { return AgableMob::GetInteractTooltip(holder); }
bool Animal::isDespawnProtected() const { return mDespawnProtected; }

void Animal::dropLeash(bool dropLeash, bool suppressDrops)
{
    setDespawnProtected();
    Entity::dropLeash(dropLeash, suppressDrops);
}

boost::shared_ptr<Entity> Animal::changeDimension(int dimension) { return Entity::changeDimension(dimension); }
