#include "game/world/generation/LayerShapeRuleDefinition.hpp"

#include "game/minigame/BasicShapeRuleDefinition.hpp"
#include "game/minigame/PatchyShapeRuleDefinition.hpp"
#include "game/minigame/RingShapeRuleDefinition.hpp"
#include "game/minigame/SpiralShapeRuleDefinition.hpp"
#include "game/minigame/StarShapeRuleDefinition.hpp"

LayerShapeRuleDefinition::LayerShapeRuleDefinition()
    : LayerAttributeRuleDefinition<LayerShape*>(static_cast<ConsoleGameRules::EGameRuleType>(48), CreateFuncImpl)
{
}

LayerShapeRuleDefinition::~LayerShapeRuleDefinition()
{
}

LayerAttributeChildRuleDefinition<LayerShape*>* LayerShapeRuleDefinition::CreateFuncImpl(ConsoleGameRules::EGameRuleType type)
{
    switch (type)
    {
    case static_cast<ConsoleGameRules::EGameRuleType>(49):
        return new BasicShapeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(50):
        return new StarShapeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(51):
        return new PatchyShapeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(52):
        return new RingShapeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(53):
        return new SpiralShapeRuleDefinition();
    }

    return NULL;
}
