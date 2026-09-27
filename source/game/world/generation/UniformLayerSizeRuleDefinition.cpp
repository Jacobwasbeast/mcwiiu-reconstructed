#include "game/world/generation/UniformLayerSizeRuleDefinition.hpp"

#include "game/core/GameRules_IntAttribute.hpp"
#include "game/core/UniformSize.hpp"

UniformLayerSizeRuleDefinition::UniformLayerSizeRuleDefinition()
    : mSize(-1)
{
    GameRules_IntAttribute* size = addIntAttribute(ConsoleGameRules::eGameRuleAttr_size, &mSize);
    size->setMin(0);
    size->setMax(254);
}

UniformLayerSizeRuleDefinition::~UniformLayerSizeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType UniformLayerSizeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_UniformSize;
}

LayerSize* UniformLayerSizeRuleDefinition::Generate() const
{
    return new UniformSize(mSize);
}
