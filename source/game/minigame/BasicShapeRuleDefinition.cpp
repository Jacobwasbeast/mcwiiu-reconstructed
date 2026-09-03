#include "game/minigame/BasicShapeRuleDefinition.hpp"

#include "game/core/CircleShape.hpp"
#include "game/core/SquareShape.hpp"
#include "game/core/TemplateShape.hpp"

BasicShapeRuleDefinition::BasicShapeRuleDefinition()
    : mShape(L"")
{
    addStringAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(59), &mShape);
}

BasicShapeRuleDefinition::~BasicShapeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType BasicShapeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(49);
}

LayerShape* BasicShapeRuleDefinition::Generate() const
{
    if (mShape.compare(L"circle") == 0)
    {
        return new CircleShape();
    }

    if (mShape.compare(L"square") == 0)
    {
        return new SquareShape();
    }

    if (mShape.compare(L"template") == 0)
    {
        return new TemplateShape();
    }

    return NULL;
}
