#ifndef GAME_CORE_COMBATTRACKER_HPP
#define GAME_CORE_COMBATTRACKER_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

class CombatEntry;
class DamageSource;
class LivingEntity;
class ClientboundChatPacket;

class CombatTracker
{
public:
    enum eLOCATION
    {
        eLOCATION_NONE = 0,
        eLOCATION_LADDER = 1,
        eLOCATION_VINES = 2,
        eLOCATION_WATER = 3
    };

    explicit CombatTracker(LivingEntity* mob);
    ~CombatTracker();
    void resetPreparedStatus();
    void prepareForDamage();
    void recheckStatus();
    CombatEntry* GetKnockoffEntry();
    eLOCATION getFallLocation(CombatEntry* entry);
    boost::shared_ptr<ClientboundChatPacket> getDeathMessagePacket();
    void recordDamage(const DamageSource* source, float damage, float fallDistance);
    boost::shared_ptr<LivingEntity> getKiller();
    int getCombatDuration();
    LivingEntity* getMob();
    bool isTakingDamage();
    bool isInCombat();
    CombatEntry* getLastEntry();

private:
    std::vector<CombatEntry*> mEntries;
    LivingEntity* mMob;
    int mLastDamageTime;
    int mCombatStartTime;
    int mCombatEndTime;
    bool mTakingDamage;
    bool mInCombat;
    int mPreparedStatus;
};

#endif
