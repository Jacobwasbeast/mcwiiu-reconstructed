#include "game/minigame/LinearBlendSizeRuleDefinition.hpp"

#include "game/core/LinearBlendSize.hpp"

LinearBlendSizeRuleDefinition::LinearBlendSizeRuleDefinition()
    : mMaxSize(-1)
    , mMinSize(-1)
{
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(68), &mMaxSize);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(69), &mMinSize);
}

LinearBlendSizeRuleDefinition::~LinearBlendSizeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType LinearBlendSizeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(47);
}

LayerSize* LinearBlendSizeRuleDefinition::Generate() const
{
    return new LinearBlendSize(mMinSize, mMaxSize);
}
