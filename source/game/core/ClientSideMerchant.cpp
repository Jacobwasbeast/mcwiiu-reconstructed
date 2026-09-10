#include "game/core/ClientSideMerchant.hpp"

#include <cstdint>

#include "game/item/MerchantContainer.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/Villager.hpp"

ClientSideMerchant::ClientSideMerchant(boost::shared_ptr<Player> player, const std::wstring& name)
    : mPosition(), mContainer(NULL), mTradingPlayer(player), mOffers(new MerchantRecipeList()), mName(name), mVillager()
{
}

ClientSideMerchant::~ClientSideMerchant()
{
    delete mContainer;
    delete mOffers;
}

MerchantContainer* ClientSideMerchant::createContainer()
{
    mContainer = new MerchantContainer(mTradingPlayer, boost::shared_ptr<Merchant>(this));
    return mContainer;
}

boost::shared_ptr<Player> ClientSideMerchant::getTradingPlayer()
{
    return mTradingPlayer;
}

void ClientSideMerchant::setTradingPlayer(const boost::shared_ptr<Player>& player)
{
}

MerchantRecipeList* ClientSideMerchant::getOffers(const boost::shared_ptr<Player>& player)
{
    return mOffers;
}

void ClientSideMerchant::overrideOffers(MerchantRecipeList* offers)
{
    delete mOffers;
    mOffers = offers;
}

void ClientSideMerchant::notifyTrade(MerchantRecipe* recipe)
{
    if (recipe != NULL)
    {
        recipe->increaseUses();
    }
}

void ClientSideMerchant::notifyTradeUpdated(const not_null_ptr<ItemInstance>& item)
{
}

std::wstring ClientSideMerchant::getDisplayName()
{
    return mName;
}

void ClientSideMerchant::setVillager(boost::shared_ptr<Villager> villager)
{
    mVillager = villager;
}

bool ClientSideMerchant::isAvailableToTrade()
{
    return !mVillager || mVillager->isAvailableToTrade();
}

Level* ClientSideMerchant::getLevel()
{
    return mTradingPlayer->mLevel;
}

BlockPos ClientSideMerchant::getPosition()
{
    return mPosition;
}

MerchantContainer* ClientSideMerchant::getContainer()
{
    return mContainer;
}
