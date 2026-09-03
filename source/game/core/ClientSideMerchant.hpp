#ifndef GAME_CORE_CLIENTSIDEMERCHANT_HPP
#define GAME_CORE_CLIENTSIDEMERCHANT_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/Merchant.hpp"
#include "game/world/block/BlockPos.hpp"

class MerchantContainer;
class MerchantRecipe;
class MerchantRecipeList;
class ItemInstance;
class Player;
class Villager;

class ClientSideMerchant : public Merchant
{
public:
    ClientSideMerchant(boost::shared_ptr<Player> player, const std::wstring& name);
    virtual ~ClientSideMerchant();

    MerchantContainer* createContainer();
    boost::shared_ptr<Player> getTradingPlayer();
    virtual void setTradingPlayer(const boost::shared_ptr<Player>& player);
    virtual MerchantRecipeList* getOffers(const boost::shared_ptr<Player>& player);
    virtual void overrideOffers(MerchantRecipeList* offers);
    virtual void notifyTrade(MerchantRecipe* recipe);
    virtual void notifyTradeUpdated(const not_null_ptr<ItemInstance>& item);
    virtual std::wstring getDisplayName();
    void setVillager(boost::shared_ptr<Villager> villager);
    virtual bool isAvailableToTrade();
    virtual int getLevel();
    virtual BlockPos getPosition();
    MerchantContainer* getContainer();

private:
    BlockPos mPosition;
    MerchantContainer* mContainer;
    boost::shared_ptr<Player> mTradingPlayer;
    MerchantRecipeList* mOffers;
    std::wstring mName;
    boost::shared_ptr<Villager> mVillager;
};

#endif
