#include "game/world/generation/LookAtTradingPlayerGoal.hpp"

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/world/generation/Villager.hpp"

class Player;

LookAtTradingPlayerGoal::LookAtTradingPlayerGoal(Villager* villager)
    : LookAtPlayerGoal(villager, typeid(Player), 8.0f), mVillager(villager)
{
}

LookAtTradingPlayerGoal::~LookAtTradingPlayerGoal()
{
}

bool LookAtTradingPlayerGoal::canUse()
{
    if (!mVillager->isTrading())
    {
        return false;
    }

    mLookAt = boost::dynamic_pointer_cast<Player>(mVillager->getTradingPlayer());
    return true;
}

std::wstring LookAtTradingPlayerGoal::getName()
{
    return std::wstring(L"LookAtTradingPlayer");
}
