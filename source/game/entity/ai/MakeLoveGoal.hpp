#ifndef GAME_ENTITY_AI_MAKELOVEGOAL_HPP
#define GAME_ENTITY_AI_MAKELOVEGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class Level;
class Villager;
class Village;

class MakeLoveGoal : public Goal
{
public:
    MakeLoveGoal(Villager* villager);
    virtual ~MakeLoveGoal();

    bool villageNeedsMoreVillagers();
    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual bool canContinueToUse();
    void breed();
    virtual void tick();
    virtual void setLevel(Level* level);
    virtual std::wstring getName();

private:
    Villager* mVillager;
    boost::shared_ptr<Village> mVillage;
    Level* mLevel;
    int mRunningTicks;
    boost::shared_ptr<Villager> mPartner;
};

#endif
