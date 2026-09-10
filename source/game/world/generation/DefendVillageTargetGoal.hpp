#ifndef GAME_WORLD_GENERATION_DEFENDVILLAGETARGETGOAL_HPP
#define GAME_WORLD_GENERATION_DEFENDVILLAGETARGETGOAL_HPP

#include <string>

#include <boost/weak_ptr.hpp>

#include "game/entity/ai/TargetGoal.hpp"

class LivingEntity;
class VillagerGolem;

class DefendVillageTargetGoal : public TargetGoal
{
public:
    DefendVillageTargetGoal(VillagerGolem* golem);
    virtual ~DefendVillageTargetGoal();

    virtual bool canUse();
    virtual void start();
    virtual std::wstring getName();

private:
    VillagerGolem* mGolem;
    boost::weak_ptr<LivingEntity> mVillageAggressorTarget;
};

#endif
