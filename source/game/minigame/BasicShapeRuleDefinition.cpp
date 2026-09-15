#include "game/minigame/BasicShapeRuleDefinition.hpp"

#include "game/core/CircleShape.hpp"
#include "game/core/SquareShape.hpp"
#include "game/core/TemplateShape.hpp"
#include "game/core/IGameRuleAttribute.hpp"

namespace
{
const ConsoleGameRules::EGameRuleType BASIC_SHAPE_ACTION_TYPE =
    static_cast<ConsoleGameRules::EGameRuleType>(49);
}

BasicShapeRuleDefinition::BasicShapeRuleDefinition()
    : mShape(L"")
{
    addStringAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(
                           IGameRuleAttribute::attributeIdFromName(L"type")), &mShape);
}

BasicShapeRuleDefinition::~BasicShapeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType BasicShapeRuleDefinition::getActionType()
{
    return BASIC_SHAPE_ACTION_TYPE;
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
