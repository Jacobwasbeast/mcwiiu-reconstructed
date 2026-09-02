#include "game/item/CollectItemRuleDefinition.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GameRule.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/network/Connection.hpp"
#include "game/network/Packet.hpp"
#include "game/network/UpdateGameRuleProgressPacket.hpp"

CollectItemRuleDefinition::CollectItemRuleDefinition()
{
    mAuxValue = 0;
    mItemId = 0;
    mQuantity = 0;

    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(5), &mItemId);
    addUnsignedCharAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(7), &mAuxValue);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(6), &mQuantity);
}

CollectItemRuleDefinition::~CollectItemRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType CollectItemRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(26);
}

void CollectItemRuleDefinition::populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule* gameRule)
{
    GameRule::_ValueType quantity;

    gameRule->setParameter(L"iQuantity", quantity);

    GameRuleDefinition::populateGameRule(type, gameRule);
}

int CollectItemRuleDefinition::getGoal()
{
    return mQuantity;
}

int CollectItemRuleDefinition::getProgress(GameRule* gameRule)
{
    return gameRule->getParameter(L"iQuantity").iValue;
}

int CollectItemRuleDefinition::getIcon()
{
    return mItemId;
}

int CollectItemRuleDefinition::getAuxValue()
{
    return mAuxValue;
}

bool CollectItemRuleDefinition::onCollectItem(GameRule* gameRule, not_null_ptr<ItemInstance> item)
{
    if (Item::getId(item->getItem()) != mItemId)
    {
        return false;
    }

    if (item->getAuxValue() != mAuxValue)
    {
        return false;
    }

    if (item->get4JData() != mDataTag)
    {
        return false;
    }

    if (getComplete(gameRule))
    {
        return false;
    }

    GameRule::_ValueType quantity = gameRule->getParameter(L"iQuantity");

    quantity.iValue += item->getCount();

    gameRule->setParameter(L"iQuantity", quantity);

    if (quantity.iValue >= mQuantity)
    {
        setComplete(gameRule, true);

        if (gameRule->mConnection != NULL)
        {
            boost::shared_ptr<Packet> packet(new UpdateGameRuleProgressPacket(getActionType(), mDescriptionName, mItemId, mAuxValue, mDataTag, NULL, 0));

            gameRule->mConnection->send(packet);
        }
    }

    return true;
}
