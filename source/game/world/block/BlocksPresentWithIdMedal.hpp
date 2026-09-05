#ifndef GAME_WORLD_BLOCK_BLOCKSPRESENTWITHIDMEDAL_HPP
#define GAME_WORLD_BLOCK_BLOCKSPRESENTWITHIDMEDAL_HPP

#include <set>

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class BlocksPresentWithIdMedal : public GameMedal
{
public:
    BlocksPresentWithIdMedal(int name, float target, unsigned int weight,
                             const std::set<int>& blockIds,
                             int (&scoreFunction)(int, int), float value,
                             GameMedal::EOperator op);

    virtual ~BlocksPresentWithIdMedal();

    virtual double GetScore(PlayerStats* playerStats) const;

    static int ABSOLUTE_VALUE(int value, int total);
    static int PERCENTAGE_OF_TOTAL(int value, int total);

private:
    std::set<int> mBlockIds;
    int (*mScoreFunction)(int, int);
};

#endif
