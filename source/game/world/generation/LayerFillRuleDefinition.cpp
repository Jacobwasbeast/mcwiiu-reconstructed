#include "game/world/generation/LayerFillRuleDefinition.hpp"

#include "game/world/generation/BasicLayerFillRuleDefinition.hpp"
#include "game/world/generation/CurvedLayerFillRuleDefinition.hpp"
#include "game/world/generation/WarpedLayerFillRuleDefinition.hpp"

LayerFillRuleDefinition::LayerFillRuleDefinition()
    : LayerAttributeRuleDefinition<LayerFill*>(static_cast<ConsoleGameRules::EGameRuleType>(54), CreateFuncImpl)
{
}

LayerFillRuleDefinition::~LayerFillRuleDefinition()
{
}

LayerAttributeChildRuleDefinition<LayerFill*>* LayerFillRuleDefinition::CreateFuncImpl(ConsoleGameRules::EGameRuleType type)
{
    switch (type)
    {
    case static_cast<ConsoleGameRules::EGameRuleType>(55):
        return new BasicLayerFillRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(56):
        return new CurvedLayerFillRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(57):
        return new WarpedLayerFillRuleDefinition();

    default:
        break;
    }

    return NULL;
}
