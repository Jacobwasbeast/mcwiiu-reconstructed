#include "game/entity/mob/TamableAnimal.hpp"

#include "game/core/DamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/SitGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"

const EntityDataAccessor<unsigned char>* TamableAnimal::DATA_FLAGS = NULL;
const EntityDataAccessor<std::wstring>* TamableAnimal::DATA_OWNER_UUID = NULL;

TamableAnimal::TamableAnimal(Level* level) : Animal(level), mSitGoal(NULL)
{
}

TamableAnimal::~TamableAnimal()
{
}

std::wstring TamableAnimal::getOwnerUUID()
{
    return mEntityData->get(DATA_OWNER_UUID);
}

boost::shared_ptr<LivingEntity> TamableAnimal::getOwner()
{
    if (!isTame()) return boost::shared_ptr<LivingEntity>();
    boost::shared_ptr<Player> player = mLevel->getPlayerByUUID(getOwnerUUID());
    return boost::static_pointer_cast<LivingEntity>(player);
}

void TamableAnimal::staticCtor()
{
    DATA_FLAGS = SynchedEntityData::defineId(0, 0x000000000D6C0000ULL, EntityDataSerializers::BYTE);
    DATA_OWNER_UUID = SynchedEntityData::defineId(0, 0x000000000D6C0000ULL, EntityDataSerializers::STRING);
}

void TamableAnimal::fjDerivedCtorCalls()
{
    Mob::fjDerivedCtorCalls();
    reassessTameGoals();
}

void TamableAnimal::defineSynchedData()
{
    Animal::defineSynchedData();
    mEntityData->createDateItem(DATA_FLAGS, static_cast<unsigned char>(0));
    mEntityData->createDateItem(DATA_OWNER_UUID, std::wstring(L""));
}

void TamableAnimal::addAdditonalSaveData(CompoundTag* tag)
{
    Animal::addAdditonalSaveData(tag);
    if (isTame()) tag->putString(L"OwnerUUID", getOwnerUUID());
    tag->putBoolean(L"Sitting", isSitting());
}

void TamableAnimal::readAdditionalSaveData(CompoundTag* tag)
{
    Animal::readAdditionalSaveData(tag);
    if (tag->contains(L"OwnerUUID")) setOwnerUUID(tag->getString(L"OwnerUUID"));
    else if (tag->contains(L"Owner")) setOwnerUUID(tag->getString(L"Owner"));
    setTame(!getOwnerUUID().empty());
    setSitting(tag->getBoolean(L"Sitting"));
}

bool TamableAnimal::canBeLeashed(const boost::shared_ptr<Player>& player)
{
    return isTame() && isOwnedBy(boost::static_pointer_cast<LivingEntity>(player));
}

void TamableAnimal::spawnTamingParticles(bool success)
{
    const ParticleType* particle = success ? PARTICLE_HEART : PARTICLE_SMOKE;

    for (int i = 0; i < 7; ++i)
    {
        mLevel->addParticle(particle,
                            mX + mRandom->nextFloat() * mBbWidth * 2.0 - mBbWidth,
                            mY + 0.5 + mRandom->nextFloat() * mBbHeight,
                            mZ + mRandom->nextFloat() * mBbWidth * 2.0 - mBbWidth,
                            mRandom->nextGaussian() * 0.02,
                            mRandom->nextGaussian() * 0.02,
                            mRandom->nextGaussian() * 0.02,
                            arrayWithLength<int>());
    }
}

void TamableAnimal::handleEntityEvent(unsigned char event, int data)
{
    Animal::handleEntityEvent(event, data);
}

bool TamableAnimal::isTame()
{
    return (mEntityData->get(DATA_FLAGS) & 4) != 0;
}

void TamableAnimal::setTame(bool tame)
{
    unsigned char flags = mEntityData->get(DATA_FLAGS);
    mEntityData->set(DATA_FLAGS, tame ? static_cast<unsigned char>(flags | 4) : static_cast<unsigned char>(flags & 251), true);
    reassessTameGoals();
}

void TamableAnimal::reassessTameGoals()
{
}

bool TamableAnimal::removeWhenFarAway()
{
    return !isTame() && Animal::removeWhenFarAway();
}

bool TamableAnimal::isSitting()
{
    return (mEntityData->get(DATA_FLAGS) & 1) != 0;
}

void TamableAnimal::setSitting(bool sitting)
{
    unsigned char flags = mEntityData->get(DATA_FLAGS);
    mEntityData->set(DATA_FLAGS, sitting ? static_cast<unsigned char>(flags | 1) : static_cast<unsigned char>(flags & 254), true);
}

void TamableAnimal::setOwnerUUID(const std::wstring& uuid)
{
    mEntityData->set(DATA_OWNER_UUID, uuid, true);
}

void TamableAnimal::tame(boost::shared_ptr<Player> player)
{
    setTame(true);
    setOwnerUUID(player->getUUID());
}

bool TamableAnimal::isOwnedBy(boost::shared_ptr<LivingEntity> entity)
{
    return entity != NULL && entity == getOwner();
}

SitGoal* TamableAnimal::getSitGoal()
{
    return mSitGoal;
}

bool TamableAnimal::wantsToAttack(boost::shared_ptr<LivingEntity> owner,
                                  boost::shared_ptr<LivingEntity> target)
{
    return true;
}

Team* TamableAnimal::getTeam()
{
    boost::shared_ptr<LivingEntity> owner = getOwner();
    return owner != NULL ? owner->getTeam() : LivingEntity::getTeam();
}

bool TamableAnimal::isAlliedTo(boost::shared_ptr<Entity> entity)
{
    boost::shared_ptr<LivingEntity> owner = getOwner();
    if (owner != NULL && owner->is(entity)) return true;
    return owner != NULL ? owner->isAlliedTo(entity) : Entity::isAlliedTo(entity);
}

void TamableAnimal::die(const DamageSource* source)
{
    LivingEntity::die(source);
}

bool TamableAnimal::interactAt(const boost::shared_ptr<Player>& player, Vec3* pos,
                               InteractionHand::EInteractionHand hand)
{
    return !isTame();
}
