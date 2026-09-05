#ifndef GAME_ENTITY_AI_BEGGOAL_HPP
#define GAME_ENTITY_AI_BEGGOAL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/Goal.hpp"

class Level;
class Player;
class Wolf;

class BegGoal : public Goal
{
public:
    BegGoal(Wolf* wolf, float distance);
    virtual ~BegGoal();

    bool playerHoldingInteresting(boost::shared_ptr<Player> player);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual void setLevel(Level* level);
    virtual std::wstring getName();

private:
    Wolf* mWolf;
    boost::shared_ptr<Player> mPlayer;
    Level* mLevel;
    float mDistance;
    int mTime;
};

#endif
