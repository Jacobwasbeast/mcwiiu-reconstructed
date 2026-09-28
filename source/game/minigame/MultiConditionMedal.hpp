#ifndef GAME_MINIGAME_MULTICONDITIONMEDAL_HPP
#define GAME_MINIGAME_MULTICONDITIONMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class MultiConditionMedal : public GameMedal
{
public:
    class MultiCondition : public GameMedal::Condition
    {
    public:
        MultiCondition(float target, GameMedal::EOperator op, bool bestOnly);
        virtual ~MultiCondition() {}

        virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
        virtual double getScore(const GameMedal* medal, PlayerStats* playerStats) const;
    };

    MultiConditionMedal(int name, unsigned int weight, MultiCondition* condition);
    virtual ~MultiConditionMedal();

    virtual bool CanAward(const StatsUID& uid, GameStats* gameStats) const;
    virtual double GetScore(PlayerStats* playerStats) const;
    MultiConditionMedal* addExtraCondition(MultiCondition* condition);
};

#endif
