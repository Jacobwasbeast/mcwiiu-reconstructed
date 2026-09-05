#include "game/core/GameCondition.hpp"

#include "game/core/MasterGameMode.hpp"

GameCondition::~GameCondition()
{
}

bool GameCondition::UpdateMessage(MasterGameMode* gameMode, int message) const
{
    std::vector<int> params;
    AddIntParams(gameMode, &params);
    params.push_back(message);
    gameMode->BroadcastChat(GetMessage(), params);
    return true;
}
