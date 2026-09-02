#include "game/world/block/UseBlockRuleDefinition.hpp"

#include "game/core/GameRule.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

UseBlockRuleDefinition::UseBlockRuleDefinition()
{
    mBlockId = -1;
    mY = 0;
    mZ = 0;
    mUsePosition = false;
    mX = 0;

    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(12), &mBlockId);
    addBoolAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(13), &mUsePosition);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(25), &mX);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(28), &mY);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(31), &mZ);
}

UseBlockRuleDefinition::~UseBlockRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType UseBlockRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(25);
}

bool UseBlockRuleDefinition::onUseBlock(GameRule* gameRule, int blockId, int x, int y, int z)
{
    if (mBlockId != blockId)
    {
        return false;
    }

    if (mUsePosition && (mX != x || mY != y || mZ != z))
    {
        return false;
    }

    if (getComplete(gameRule))
    {
        return false;
    }

    setComplete(gameRule, true);

    return true;
}
