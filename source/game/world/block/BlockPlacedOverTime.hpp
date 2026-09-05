#ifndef GAME_WORLD_BLOCK_BLOCKPLACEDOVERTIME_HPP
#define GAME_WORLD_BLOCK_BLOCKPLACEDOVERTIME_HPP

#include <map>

#include "game/minigame/GameMedal.hpp"

class BlockPlacedOverTime : public GameMedal
{
public:
    typedef double ScoreFunction(int, const std::map<int, int>&);

    BlockPlacedOverTime(int name, float target, unsigned int weight, ScoreFunction& scoreFunction, GameMedal::EOperator op);
    virtual ~BlockPlacedOverTime();

    static double START_DELAY(int roundTime, const std::map<int, int>& blocksPlaced);
    static double TIME_LEFTOVER(int roundTime, const std::map<int, int>& blocksPlaced);
    static double IN_FIRST_10SECS(int roundTime, const std::map<int, int>& blocksPlaced);
    static double IN_LAST_10SECS(int roundTime, const std::map<int, int>& blocksPlaced);

    virtual double GetScore(PlayerStats* playerStats) const;

private:
    ScoreFunction* mScoreFunction;
};

#endif
