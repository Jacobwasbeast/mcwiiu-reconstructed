#include "game/entity/ai/HurtByTargetGoal.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/EntityType.hpp"
#include "game/entity/ai/TargetGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

HurtByTargetGoal::HurtByTargetGoal(PathfinderMob* mob, bool alertSameType,
                                   arrayWithLength<unsigned long long> entityTypes)
    : TargetGoal(mob, alertSameType),
      mAlertSameType(alertSameType),
      mTimestamp(0),
      mEntityTypes(NULL),
      mEntityTypesLength(0)
{
    mEntityTypes = entityTypes.data;
    mEntityTypesLength = entityTypes.length;
    setRequiredControlFlags(1);
}

HurtByTargetGoal::~HurtByTargetGoal()
{
    delete[] mEntityTypes;
}

bool HurtByTargetGoal::canUse()
{
    if (mMob->getLastHurtByMobTimestamp() == mTimestamp)
        return false;

    boost::shared_ptr<LivingEntity> attacker = mMob->getLastHurtByMob();
    if (!attacker)
        return false;

    if (!canAttack(attacker, false))
        return false;

    return !mMob->mRemoved;
}

void HurtByTargetGoal::alertOthers()
{
    double followDistance = getFollowDistance();
    AABB* box = AABB::newTemp(mMob->mX, mMob->mY, mMob->mZ, mMob->mX + 1.0, mMob->mY + 1.0, mMob->mZ + 1.0)
                    ->grow(followDistance, 4.0, followDistance);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mMob->mLevel->getEntitiesOfClass(typeid(*mMob), box);

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
         it != entities->end(); ++it)
    {
        boost::shared_ptr<PathfinderMob> mob =
            boost::dynamic_pointer_cast<PathfinderMob>(*it);
        if (!mob || mob.get() == mMob || mob->getTarget())
            continue;

        if ((mMob->GetType() & EntityType::TAMABLE_ANIMAL) == EntityType::TAMABLE_ANIMAL
            && (mob->GetType() & EntityType::TAMABLE_ANIMAL) == EntityType::TAMABLE_ANIMAL
            && static_cast<TamableAnimal*>(mMob)->getOwner() != boost::dynamic_pointer_cast<TamableAnimal>(mob)->getOwner())
            continue;

        if (mob->isAlliedTo(mMob->getLastHurtByMob()))
            continue;

        bool excluded = false;
        unsigned int i;
        for (i = 0; i < mEntityTypesLength; ++i)
        {
            if (mob->GetType() == mEntityTypes[i])
            {
                excluded = true;
                break;
            }
        }
        if (excluded)
            continue;

        alertOther(mob, mMob->getLastHurtByMob());
    }

    delete entities;
}

void HurtByTargetGoal::start()
{
    mMob->setTarget(mMob->getLastHurtByMob());
    mTarget = mMob->getTarget();
    mTimestamp = mMob->getLastHurtByMobTimestamp();
    mUnseenMemoryTicks = 300;
    if (mAlertSameType)
        alertOthers();
    TargetGoal::start();
}

void HurtByTargetGoal::alertOther(boost::shared_ptr<PathfinderMob> mob,
                                  boost::shared_ptr<LivingEntity> target)
{
    if (mob)
        mob->setTarget(target);
}

std::wstring HurtByTargetGoal::getName()
{
    return std::wstring(L"HurtByTarget");
}
