#include "game/world/generation/TradeWithPlayerGoal.hpp"

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/Villager.hpp"

TradeWithPlayerGoal::TradeWithPlayerGoal(Villager* villager)
    : Goal(), mVillager(villager)
{
    setRequiredControlFlags(5);
}

TradeWithPlayerGoal::~TradeWithPlayerGoal()
{
}

bool TradeWithPlayerGoal::canUse()
{
    if (!mVillager->isAlive())
    {
        return false;
    }
    if (mVillager->isInWater())
    {
        return false;
    }
    if (!mVillager->mOnGround || mVillager->mHurtMarked)
    {
        return false;
    }

    boost::shared_ptr<Player> tradingPlayer = mVillager->getTradingPlayer();
    if (!tradingPlayer)
    {
        return false;
    }
    if (mVillager->distanceToSqr(boost::static_pointer_cast<Entity>(tradingPlayer)) > 16.0)
    {
        return false;
    }
    return tradingPlayer->mContainerMenu != NULL;
}

void TradeWithPlayerGoal::start()
{
    mVillager->getNavigation()->stop();
}

void TradeWithPlayerGoal::stop()
{
    mVillager->setTradingPlayer(boost::shared_ptr<Player>());
}

std::wstring TradeWithPlayerGoal::getName()
{
    return std::wstring(L"TradeWithPlayer");
}
