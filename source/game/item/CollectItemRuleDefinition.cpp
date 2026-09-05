#include "game/item/CollectItemRuleDefinition.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/GameRule.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/Connection.hpp"
#include "game/network/UpdateGameRuleProgressPacket.hpp"

CollectItemRuleDefinition::CollectItemRuleDefinition()
    : GameRuleDefinition()
    , mIcon(0)
    , mAuxValue(0)
    , mGoal(0)
{
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(5), &mIcon);
    addUnsignedCharAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(7), &mAuxValue);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(6), &mGoal);
}

CollectItemRuleDefinition::~CollectItemRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType CollectItemRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(26);
}

int CollectItemRuleDefinition::getGoal()
{
    return mGoal;
}

int CollectItemRuleDefinition::getProgress(GameRule* gameRule)
{
    return gameRule->getParameter(L"iQuantity").iValue;
}

int CollectItemRuleDefinition::getIcon()
{
    return mIcon;
}

int CollectItemRuleDefinition::getAuxValue()
{
    return mAuxValue;
}

void CollectItemRuleDefinition::populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule* gameRule)
{
    GameRule::_ValueType value;
    gameRule->setParameter(L"iQuantity", value);
    GameRuleDefinition::populateGameRule(type, gameRule);
}

bool CollectItemRuleDefinition::onCollectItem(GameRule* gameRule, not_null_ptr<ItemInstance> item)
{
    if (Item::getId(item->getItem()) != mIcon)
        return false;
    if (item->getAuxValue() != mAuxValue)
        return false;
    if (item->get4JData() != mDataTag)
        return false;
    if (getComplete(gameRule))
        return false;

    GameRule::_ValueType value = gameRule->getParameter(L"iQuantity");
    value.iValue += item->getCount();
    gameRule->setParameter(L"iQuantity", value);

    if (mGoal <= value.iValue)
    {
        setComplete(gameRule, true);
        if (gameRule->mConnection != 0)
        {
            boost::shared_ptr<Packet> packet(new UpdateGameRuleProgressPacket(
                getActionType(), mDescriptionName, mIcon, mAuxValue, mDataTag, 0, 0));
            gameRule->mConnection->send(packet);
        }
    }

    return true;
}
