#include "game/minigame/MedalAdder.hpp"

#include "game/minigame/GameMedal.hpp"

MedalAdder::MedalAdder(int* nextMedalId, std::vector<const GameMedal*>* medals, std::vector<const GameMedal*>* allMedals)
    : mNextMedalId(nextMedalId)
    , mMedals(medals)
    , mAllMedals(allMedals)
{
}

void MedalAdder::addMedal(GameMedal* medal)
{
    medal->SetId(*mNextMedalId);
    mMedals->push_back(medal);
    mAllMedals->push_back(medal);
    ++(*mNextMedalId);
}
