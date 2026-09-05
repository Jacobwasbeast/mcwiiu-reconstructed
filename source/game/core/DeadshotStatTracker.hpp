#ifndef GAME_CORE_DEADSHOTSTATTRACKER_HPP
#define GAME_CORE_DEADSHOTSTATTRACKER_HPP

#include "game/core/UntypedStatTracker.hpp"

class DamageEvent;

class DeadshotStatTracker : public UntypedStatTracker
{
public:
    DeadshotStatTracker(const StatsUID& statsUID);
    virtual ~DeadshotStatTracker();

    virtual void update(const DamageEvent* damageEvent);

private:
    bool mHit;
    int mFallCount;
};

#endif
