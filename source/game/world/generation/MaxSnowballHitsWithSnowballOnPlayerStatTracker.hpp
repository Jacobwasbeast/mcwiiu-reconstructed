#ifndef GAME_WORLD_GENERATION_MAXSNOWBALLHITSWITHSNOWBALLONPLAYERSTATTRACKER_HPP
#define GAME_WORLD_GENERATION_MAXSNOWBALLHITSWITHSNOWBALLONPLAYERSTATTRACKER_HPP

#include <functional>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/UntypedStatTracker.hpp"
#include "game/world/generation/PlayerUID.hpp"

class DamageEvent;

class MaxSnowballHitsWithSnowballOnPlayerStatTracker : public UntypedStatTracker
{
public:
    MaxSnowballHitsWithSnowballOnPlayerStatTracker(const StatsUID& statsUID);
    virtual ~MaxSnowballHitsWithSnowballOnPlayerStatTracker();
    virtual void update(const DamageEvent* damageEvent);
    unsigned int getValue() const;

private:
    unsigned int mMaxSnowballHits;
    PlayerUID mPlayerUID;
    boost::unordered_map<StatsUID, unsigned int, boost::hash<StatsUID>, std::equal_to<StatsUID> > mSnowballHits;
};

#endif
