#include "game/core/CombatEntry.hpp"

#include "game/core/DamageSource.hpp"
#include "game/entity/mob/Entity.hpp"

CombatEntry::CombatEntry(const DamageSource* source, int time, float healthBeforeDamage, float damage, CombatTracker::eLOCATION location, float fallDistance)
{
    mSource = NULL;

    if (source != NULL)
    {
        mSource = new DamageSource(*source);
    }

    mTime = time;
    mDamage = damage;
    mHealthBeforeDamage = healthBeforeDamage;
    mLocation = location;
    mFallDistance = fallDistance;
}

CombatEntry::~CombatEntry()
{
    delete mSource;
}

bool CombatEntry::isCombatRelated()
{
    return mSource->getEntity() && (mSource->getEntity()->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY;
}

const DamageSource* CombatEntry::getSource()
{
    return mSource;
}

int CombatEntry::getTime()
{
    return mTime;
}

CombatTracker::eLOCATION CombatEntry::getLocation()
{
    return mLocation;
}

float CombatEntry::getDamage()
{
    return mDamage;
}

float CombatEntry::getHealthAfterDamage()
{
    return mHealthBeforeDamage - mDamage;
}

float CombatEntry::getHealthBeforeDamage()
{
    return mHealthBeforeDamage;
}

float CombatEntry::getFallDistance()
{
    return mFallDistance;
}

std::wstring CombatEntry::getAttackerName()
{
    return getSource()->getEntity() ? getSource()->getEntity()->getNetworkName() : L"";
}

boost::shared_ptr<Entity> CombatEntry::getAttacker()
{
    return getSource()->getEntity();
}
