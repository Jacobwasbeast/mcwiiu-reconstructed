#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Wolf.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"

namespace
{
}


const EntityDataAccessor<int>* Animal::DATA_IN_LOVE = NULL;
const EntityDataAccessor<int>* Animal::DATA_IN_LOVE_TIME = NULL;

Animal::Animal(Level* level) : AgableMob(level), mSpawnableBlock(Blocks::grass), mLoveCause(L""),
                               mBreedCooldown(1200)
{
    setDespawnProtected();
    mCategories |= EntityCategory::eEntityCategory_Animal;
}

void Animal::staticCtor()
{
    DATA_IN_LOVE = SynchedEntityData::defineId(EntityType::ANIMAL, EntityType::MOB,
                                               EntityDataSerializers::INT);
    DATA_IN_LOVE_TIME = SynchedEntityData::defineId(EntityType::ANIMAL, EntityType::MOB,
                                                    EntityDataSerializers::INT);
}

void Animal::setDespawnProtected()
{
    if (mLevel != NULL && mLevel->mIsClientSide)
    {
        return;
    }

    int x = Mth::floor(mX);
    int z = Mth::floor(mZ);

    mMinX = static_cast<short>(x);
    mMaxX = static_cast<short>(x);
    mDespawnProtected = true;
    mMinZ = static_cast<short>(z);
    mMaxZ = static_cast<short>(z);
}

void Animal::defineSynchedData()
{
    AgableMob::defineSynchedData();
    mEntityData->createDateItem(DATA_IN_LOVE, 0);
    mEntityData->createDateItem(DATA_IN_LOVE_TIME, 0);
}

void Animal::setInLoveValue(int value)
{
    if (!mLevel->mIsClientSide)
    {
        mEntityData->set(DATA_IN_LOVE, value, true);
    }
}

void Animal::customServerAiStep()
{
    if (isBaby())
    {
        setInLoveValue(0);
    }

    Mob::customServerAiStep();
}

int Animal::getInLoveValue()
{
    return mEntityData->get(DATA_IN_LOVE);
}

void Animal::updateDespawnProtectedState()
{
    if (mLevel->mIsClientSide)
    {
        return;
    }

    if (!mDespawnProtected)
    {
        return;
    }

    int x = Mth::floor(mX);
    int z = Mth::floor(mZ);

    if (mMaxX < x)
    {
        mMaxX = static_cast<short>(x);
    }

    if (x < mMinX)
    {
        mMinX = static_cast<short>(x);
    }

    if (mMaxZ < z)
    {
        mMaxZ = static_cast<short>(z);
    }

    if (z < mMinZ)
    {
        mMinZ = static_cast<short>(z);
    }

    if (mMaxX - mMinX > 20 || mMaxZ - mMinZ > 20)
    {
        mDespawnProtected = false;
    }
}

void Animal::aiStep()
{
    AgableMob::aiStep();

    if (isBaby())
    {
        setInLoveValue(0);
    }

    if (getInLoveValue() > 0)
    {
        setInLoveValue(getInLoveValue() - 1);
        if (getInLoveValue() % 10 == 0)
        {
            mLevel->addParticle(PARTICLE_HEART,
                                mX + mRandom->nextFloat() * mBbWidth * 2.0 - mBbWidth,
                                mY + 0.5 + mRandom->nextFloat() * mBbHeight,
                                mZ + mRandom->nextFloat() * mBbWidth * 2.0 - mBbWidth,
                                mRandom->nextGaussian() * 0.02,
                                mRandom->nextGaussian() * 0.02,
                                mRandom->nextGaussian() * 0.02,
                                arrayWithLength<int>());
        }
    }

    if (mEntityData->get(DATA_IN_LOVE_TIME) > 0)
    {
        mEntityData->set(DATA_IN_LOVE_TIME, mEntityData->get(DATA_IN_LOVE_TIME) - 1, true);
        setInLoveValue(0);
    }

    updateDespawnProtectedState();
}

bool Animal::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
    {
        return false;
    }

    boost::shared_ptr<Entity> attacker = source->getEntity();
    bool attackDenied = false;

    if (attacker != NULL && (attacker->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(attacker);
        if (!player->isAllowedToAttackAnimals())
        {
            attackDenied = true;
        }
    }

    if (attackDenied)
    {
        return false;
    }

    setInLoveValue(0);
    return LivingEntity::hurt(source, damage);
}

float Animal::getWalkTargetValue(const BlockPos& pos)
{
    if (mLevel->getBlockState(pos.below())->getBlock() == mSpawnableBlock)
    {
        return 10.0f;
    }

    return mLevel->getBrightness(pos) - 0.5f;
}

void Animal::addAdditonalSaveData(CompoundTag* tag)
{
    AgableMob::addAdditonalSaveData(tag);
    tag->putInt(L"InLove", getInLoveValue());

    if (!mLoveCause.empty())
    {
        tag->putString(L"LoveCause", mLoveCause);
    }
}

double Animal::getRidingHeight()
{
    return 0.14;
}

void Animal::readAdditionalSaveData(CompoundTag* tag)
{
    AgableMob::readAdditionalSaveData(tag);
    setInLoveValue(tag->getInt(L"InLove"));

    if (tag->contains(L"LoveCause"))
    {
        mLoveCause = tag->getString(L"LoveCause");
    }
    else
    {
        mLoveCause = L"";
    }

    setDespawnProtected();
}

bool Animal::checkSpawnRules()
{
    BlockPos pos(Mth::floor(mX), Mth::floor(mBoundingBox->mMinY), Mth::floor(mZ));
    return mLevel->getBlockState(pos.below())->getBlock() == mSpawnableBlock
        && mLevel->getDaytimeRawBrightness(pos) > 8
        && PathfinderMob::checkSpawnRules();
}

int Animal::getAmbientSoundInterval()
{
    return 120;
}

bool Animal::removeWhenFarAway()
{
    return !isDespawnProtected() && !isLeashed() && !hasCustomName();
}

int Animal::getExperienceReward(boost::shared_ptr<Player> player)
{
    return mLevel->mRandom->nextInt(3) + 1;
}

bool Animal::isFood(const not_null_ptr<ItemInstance>& itemInstance)
{
    return itemInstance->getItem() == Items::wheat;
}

bool Animal::isBreedCoolingDown()
{
    return mEntityData->get(DATA_IN_LOVE_TIME) != 0;
}

void Animal::usePlayerItem(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance)
{
    if (!player->mAbilities.mInstabuild)
    {
        itemInstance->shrink(1);
    }
}

bool Animal::AgeUpBaby(const boost::shared_ptr<Player>& player, not_null_ptr<ItemInstance> itemInstance)
{
    if (!isBaby() || !isFood(itemInstance))
    {
        return false;
    }

    usePlayerItem(player, itemInstance);
    ageUp(6 * 20, true);
    return true;
}

bool Animal::mobInteract(const boost::shared_ptr<Player>& player,
                         InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!itemInstance->isEmpty())
    {
        if (isFood(itemInstance) && !isBaby() && getInLoveValue() <= 0 && !isBreedCoolingDown())
        {
            if (mLevel->mIsClientSide)
            {
                return false;
            }

            if (GetType() == MobCategory::MUSHROOM_COW->getEnumBaseClass())
            {
                if (!mLevel->canCreateMore(MobCategory::MUSHROOM_COW->getEnumBaseClass(), Level::eSpawnType_Breed))
                {
                    player->displayClientMessage(MESSAGE_ANIMAL_LIMIT, true);
                    return false;
                }
            }
            else if (GetType() == MobCategory::WOLF->getEnumBaseClass())
            {
                if (!mLevel->canCreateMore(MobCategory::WOLF->getEnumBaseClass(), Level::eSpawnType_Breed))
                {
                    player->displayClientMessage(MESSAGE_WOLF_LIMIT, true);
                    return false;
                }
            }
            else if (GetType() == MobCategory::CHICKEN->getEnumBaseClass())
            {
                if (!mLevel->canCreateMore(MobCategory::CHICKEN->getEnumBaseClass(), Level::eSpawnType_Breed))
                {
                    player->displayClientMessage(MESSAGE_CHICKEN_LIMIT, true);
                    return false;
                }
            }
            else if ((GetType() & EntityType::CREATURE_CATEGORY) != 0)
            {
                if (!mLevel->canCreateMore(GetType(), Level::eSpawnType_Breed))
                {
                    player->displayClientMessage(MESSAGE_MOB_LIMIT, true);
                    return false;
                }
            }

            usePlayerItem(player, itemInstance);
            setInLove(player);
            return true;
        }

        if (AgeUpBaby(player, itemInstance))
        {
            return true;
        }
    }

    return AgableMob::mobInteract(player, hand);
}

void Animal::setInLove(boost::shared_ptr<Player> player)
{
    setInLoveValue(600);

    if (player != NULL)
    {
        mLoveCause = player->getUUID();
    }

    mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::IN_LOVE_HEARTS, 0);
}

boost::shared_ptr<Player> Animal::getLoveCause()
{
    if (mLoveCause.empty())
    {
        return boost::shared_ptr<Player>();
    }

    boost::shared_ptr<Player> player = mLevel->getPlayerByUUID(mLoveCause);
    if (player != NULL && player->GetType() == EntityType::SERVER_PLAYER)
    {
        return boost::static_pointer_cast<ServerPlayer>(player);
    }

    return boost::shared_ptr<Player>();
}

bool Animal::isInLove()
{
    return mEntityData->get(DATA_IN_LOVE) != 0;
}

void Animal::resetLove()
{
    mEntityData->set(DATA_IN_LOVE, 0, true);
    mEntityData->set(DATA_IN_LOVE_TIME, mBreedCooldown, true);
}

bool Animal::canMate(const boost::shared_ptr<Animal>& other)
{
    if (other == shared_from_this())
    {
        return false;
    }

    if (GetType() != other->GetType())
    {
        return false;
    }

    return isInLove() && other->isInLove();
}

void Animal::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::IN_LOVE_HEARTS)
    {
        for (int i = 0; i < 7; ++i)
        {
            mLevel->addParticle(PARTICLE_HEART,
                                mX + mRandom->nextFloat() * mBbWidth * 2.0 - mBbWidth,
                                mY + 0.5 + mRandom->nextFloat() * mBbHeight,
                                mZ + mRandom->nextFloat() * mBbWidth * 2.0 - mBbWidth,
                                mRandom->nextGaussian() * 0.02,
                                mRandom->nextGaussian() * 0.02,
                                mRandom->nextGaussian() * 0.02,
                                arrayWithLength<int>());
        }

        return;
    }

    Mob::handleEntityEvent(event, data);
}

unsigned int Animal::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (isFood(holder.m_pItemInstance))
    {
        if (isBaby())
        {
            return IDS_TOOLTIPS_FEED;
        }

        if (!isInLove() && !isBreedCoolingDown() && !isBaby())
        {
            return IDS_TOOLTIPS_LOVEMODE;
        }
    }

    if (Item::getId(holder.m_pItemInstance->getItem()) == ItemId::LEAD && !isLeashed())
    {
        return IDS_TOOLTIPS_LEASH;
    }

    return AgableMob::GetInteractTooltip(holder);
}

bool Animal::isDespawnProtected()
{
    return mDespawnProtected;
}

void Animal::dropLeash(bool dropLeash, bool suppressDrops)
{
    setDespawnProtected();
    Entity::dropLeash(dropLeash, suppressDrops);
}

boost::shared_ptr<Entity> Animal::changeDimension(int dimension)
{
    boost::shared_ptr<Entity> entity = Entity::changeDimension(dimension);

    if (Entity::instanceof(entity, EntityType::ANIMAL))
    {
        boost::static_pointer_cast<Animal>(entity)->setDespawnProtected();
    }

    return entity;
}
