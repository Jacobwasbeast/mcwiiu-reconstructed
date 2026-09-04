#include "game/world/generation/UniformLayerSizeRuleDefinition.hpp"

#include "game/core/GameRules_IntAttribute.hpp"
#include "game/core/UniformSize.hpp"

UniformLayerSizeRuleDefinition::UniformLayerSizeRuleDefinition()
    : mSize(-1)
{
    GameRules_IntAttribute* size = addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(64), &mSize);
    size->setMin(0);
    size->setMax(254);
}

UniformLayerSizeRuleDefinition::~UniformLayerSizeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType UniformLayerSizeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(45);
}

LayerSize* UniformLayerSizeRuleDefinition::Generate() const
{
    return new UniformSize(mSize);
}
