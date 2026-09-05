#ifndef GAME_ENTITY_AI_TAKEFLOWERGOAL_HPP
#define GAME_ENTITY_AI_TAKEFLOWERGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class Villager;
class VillagerGolem;

class TakeFlowerGoal : public Goal
{
public:
    explicit TakeFlowerGoal(Villager* villager);
    virtual ~TakeFlowerGoal();

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName();

private:
    Villager* mVillager;
    boost::shared_ptr<VillagerGolem> mGolem;
    int mTicks;
    bool mWantsToTakeFlower;
};

#endif
