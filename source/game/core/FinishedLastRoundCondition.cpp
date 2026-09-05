#include "game/core/FinishedLastRoundCondition.hpp"

#include "game/core/Minecraft.hpp"

class GameStats
{
public:
    PlayerStats* GetPlayerStats(const StatsUID& uid);
};

class MiniGameDef
{
public:
    int GetFinalRoundIndex() const;
};

FinishedLastRoundCondition::FinishedLastRoundCondition()
    : MultiConditionMedal::MultiCondition(0.0f, GameMedal::OPERATOR_EQUAL, false)
{
}

FinishedLastRoundCondition::~FinishedLastRoundCondition()
{
}

bool FinishedLastRoundCondition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats == NULL)
    {
        return false;
    }

    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    return playerStats->getReachedEnd(miniGame->GetFinalRoundIndex()) != 0;
}
