#include "game/minigame/HoarderMedal.hpp"

#include <map>

#include "game/world/generation/PlayerStats.hpp"

HoarderMedal::~HoarderMedal()
{
}

double HoarderMedal::GetScore(PlayerStats* playerStats) const
{
    std::map<int, int> items;
    playerStats->getAllItems(items);
    return (float)items.size();
}
