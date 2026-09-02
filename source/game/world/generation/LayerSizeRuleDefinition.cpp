#include "game/world/generation/LayerSizeRuleDefinition.hpp"

#include "game/minigame/LinearBlendSizeRuleDefinition.hpp"
#include "game/minigame/RandomizeSizeRuleDefinition.hpp"
#include "game/world/generation/UniformLayerSizeRuleDefinition.hpp"

LayerSizeRuleDefinition::LayerSizeRuleDefinition()
    : LayerAttributeRuleDefinition<LayerSize*>(static_cast<ConsoleGameRules::EGameRuleType>(44), CreateFuncImpl)
{
}

LayerSizeRuleDefinition::~LayerSizeRuleDefinition()
{
}

LayerAttributeChildRuleDefinition<LayerSize*>* LayerSizeRuleDefinition::CreateFuncImpl(ConsoleGameRules::EGameRuleType type)
{
    switch (type)
    {
    case static_cast<ConsoleGameRules::EGameRuleType>(45):
        return new UniformLayerSizeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(46):
        return new RandomizeSizeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(47):
        return new LinearBlendSizeRuleDefinition();

    default:
        break;
    }

    return NULL;
}
