#include "game/minigame/DamageSourceMedal.hpp"

#include "game/core/StatTracker.hpp"
#include "game/world/generation/PlayerStats.hpp"

DamageSourceMedal::DamageSourceMedal(int name, float target, unsigned int weight,
                                     Scoreboard::EDamageSource source)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
    , mSources(new std::vector<Scoreboard::EDamageSource>())
{
    mSources->push_back(source);
}

DamageSourceMedal::DamageSourceMedal(int name, float target, unsigned int weight,
                                     std::vector<Scoreboard::EDamageSource>* sources)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
    , mSources(sources)
{
}

DamageSourceMedal::~DamageSourceMedal()
{
    delete mSources;
}

double DamageSourceMedal::GetScore(PlayerStats* playerStats) const
{
    double score = 0.0;

    for (std::vector<Scoreboard::EDamageSource>::const_iterator source = mSources->begin();
         source != mSources->end(); ++source)
    {
        score = static_cast<double>(static_cast<float>(
            score + static_cast<double>(static_cast<StatTracker<float>*>(
                playerStats->mDamageTakenBySource[*source])->getValue())));
    }

    return score;
}
