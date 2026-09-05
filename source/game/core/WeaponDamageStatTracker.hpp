#ifndef GAME_CORE_WEAPONDAMAGESTATTRACKER_HPP
#define GAME_CORE_WEAPONDAMAGESTATTRACKER_HPP

#include <map>

#include "game/core/Scoreboard.hpp"
#include "game/core/UntypedStatTracker.hpp"

class DamageEvent;

class WeaponDamageStatTracker : public UntypedStatTracker
{
public:
    enum ETrackingType
    {
        TRACKING_DEALT,
        TRACKING_RECEIVED
    };

    WeaponDamageStatTracker(const StatsUID& statsUID, ETrackingType trackingType);
    virtual ~WeaponDamageStatTracker();

    virtual void update(const DamageEvent* damageEvent);

private:
    Scoreboard::Weapon mWeapon;
    std::map<unsigned int, float> mDamageByWeapon;
    ETrackingType mTrackingType;
};

#endif
