#include "game/world/generation/BasicLayerFillRuleDefinition.hpp"

#include "game/world/generation/FunctionLayerFill.hpp"

BasicLayerFillRuleDefinition::BasicLayerFillRuleDefinition()
    : mType(L"")
{
    addStringAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(59), &mType);
}

BasicLayerFillRuleDefinition::~BasicLayerFillRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType BasicLayerFillRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(55);
}

LayerFill* BasicLayerFillRuleDefinition::Generate() const
{
    if (mType == L"ReverseCone")
    {
        return new FunctionLayerFill(FunctionLayerFill::ISLAND);
    }

    if (mType == L"Uniform1")
    {
        return new FunctionLayerFill(FunctionLayerFill::UNIFORM);
    }

    return NULL;
}
