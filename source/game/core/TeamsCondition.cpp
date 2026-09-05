#include "game/core/TeamsCondition.hpp"

#include "game/core/MasterGameMode.hpp"

TeamsCondition::~TeamsCondition()
{
}

int TeamsCondition::GetValue(MasterGameMode* gameMode) const
{
    return gameMode->GetActiveTeamCount();
}

void TeamsCondition::AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const
{
    params->push_back(gameMode->GetActiveTeamCount());
    params->push_back(0x137f436f);
}
