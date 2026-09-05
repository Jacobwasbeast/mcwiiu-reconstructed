#include "game/minigame/RingsMedal.hpp"

#include <cstring>
#include <vector>

#include "game/core/StatsUID.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"

class LevelRuleset;
class LevelGenerationOptions
{
public:
    LevelRuleset* getRequiredGameRules();
};

class CMinecraftApp
{
public:
    LevelGenerationOptions* getLevelGenerationOptions();
};

extern CMinecraftApp gApp;

class MiniGameDef
{
public:
    int GetGameVariant() const;
};

class Minecraft
{
public:
    static Minecraft* GetInstance();
    const MiniGameDef* GetMiniGame();
};

class LevelRuleset
{
public:
    void getTargetAreas(std::vector<TargetAreaRuleDefinition*>* areas);
};

class GameStats
{
public:
    PlayerStats* GetPlayerStats(const StatsUID& uid);
};

RingsMedal::RingsMedal(int name, unsigned int weight, eRingMedalAmount* amounts, EOperator op)
    : GameMedal(name, 0.0f, weight, op)
{
    std::memcpy(&mFirstAmount, amounts, sizeof(eRingMedalAmount) * 3);
}

RingsMedal::~RingsMedal()
{
}

bool RingsMedal::CanAward(const StatsUID& uid, GameStats* gameStats) const
{
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options == NULL)
        return false;

    LevelRuleset* ruleset = options->getRequiredGameRules();
    if (ruleset == NULL)
        return false;

    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (miniGame->GetGameVariant() != 1)
        return false;

    std::vector<TargetAreaRuleDefinition*> targetAreas;
    ruleset->getTargetAreas(&targetAreas);

    int targetAreaCounts[3] = { 0, 0, 0 };
    std::vector<TargetAreaRuleDefinition*>::iterator targetArea = targetAreas.begin();
    for (; targetArea != targetAreas.end(); ++targetArea)
    {
        if (!(*targetArea)->mPrimary)
            ++targetAreaCounts[(*targetArea)->getActionType()];
    }

    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats == NULL)
        return false;

    unsigned int totalRounds = playerStats->getTotalRounds();
    for (unsigned int round = 0; round < totalRounds; ++round)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        bool first = false;
        bool second = false;
        bool third = false;

        if (mFirstAmount == 0)
            first = roundStats->mDistanceStats[0] + roundStats->mProgressStats[0] == 0;
        else if (mFirstAmount == 1)
            first = roundStats->mDistanceStats[0] + roundStats->mProgressStats[0] == targetAreaCounts[0];
        else if (mFirstAmount < 3)
            first = roundStats->mDistanceStats[0] + roundStats->mProgressStats[0] != 0;
        else if (mFirstAmount == 3)
            first = true;
        else
            return false;

        if (mSecondAmount == 0)
            second = roundStats->mDistanceStats[1] + roundStats->mProgressStats[1] == 0;
        else if (mSecondAmount == 1)
            second = roundStats->mDistanceStats[1] + roundStats->mProgressStats[1] == targetAreaCounts[1];
        else if (mSecondAmount < 3)
            second = roundStats->mDistanceStats[1] + roundStats->mProgressStats[1] != 0;
        else if (mSecondAmount == 3)
            second = true;
        else
            return false;

        if (mThirdAmount == 0)
            third = roundStats->mDistanceStats[2] + roundStats->mProgressStats[2] == 0;
        else if (mThirdAmount == 1)
            third = roundStats->mDistanceStats[2] + roundStats->mProgressStats[2] == targetAreaCounts[2];
        else if (mThirdAmount < 3)
            third = roundStats->mDistanceStats[2] + roundStats->mProgressStats[2] != 0;
        else if (mThirdAmount == 3)
            third = true;
        else
            return false;

        if (first && second && third)
            return true;
    }

    return false;
}
