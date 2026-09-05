#include "game/minigame/FirstToDieMedal.hpp"

FirstToDieMedal::FirstToDieMedal(int name, float target, unsigned int weight)
    : GameMedal(name, target, weight, GameMedal::OPERATOR_AT_MOST)
{
}

FirstToDieMedal::~FirstToDieMedal()
{
}

double FirstToDieMedal::GetScore(PlayerStats* playerStats) const
{
    const float initialScore = 10.0f;
    const float decay = 0.95f;
    double score = 0.0;
    int winningRounds = 0;
    float roundScore = initialScore;
    unsigned int round = 0;

    while (round < playerStats->getTotalRounds())
    {
        if (playerStats->getRoundStats(round)->mWon == 1)
        {
            score = (double)(float)(score + (double)roundScore);
            ++winningRounds;
        }
        roundScore = (float)((double)roundScore * (double)decay);
        ++round;
    }

    if (winningRounds < 2)
    {
        score = 0.0;
    }

    return score;
}
