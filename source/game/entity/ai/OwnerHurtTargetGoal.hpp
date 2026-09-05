#ifndef GAME_ENTITY_AI_OWNERHURTTARGETGOAL_HPP
#define GAME_ENTITY_AI_OWNERHURTTARGETGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/entity/ai/TargetGoal.hpp"

class LivingEntity;
class TamableAnimal;

class OwnerHurtTargetGoal : public TargetGoal
{
public:
    OwnerHurtTargetGoal(TamableAnimal* tamable);
    virtual ~OwnerHurtTargetGoal();
    virtual bool canUse();
    virtual void start();
    virtual std::wstring getName();

private:
    TamableAnimal* mTamable;
    boost::shared_ptr<LivingEntity> mOwnerLastHurt;
    int mTimestamp;
};

#endif
