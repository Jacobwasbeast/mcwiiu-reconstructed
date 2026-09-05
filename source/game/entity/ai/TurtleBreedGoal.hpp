#ifndef GAME_ENTITY_AI_TURTLEBREEDGOAL_HPP
#define GAME_ENTITY_AI_TURTLEBREEDGOAL_HPP

#include <string>

#include "game/entity/ai/BreedGoal.hpp"

class Turtle;

class TurtleBreedGoal : public BreedGoal
{
public:
    TurtleBreedGoal(Turtle* turtle, double speedModifier);
    virtual ~TurtleBreedGoal();
    virtual bool canUse();
    virtual void breed();
    virtual std::wstring getName();

private:
    Turtle* mTurtle;
};

#endif
