#include "game/world/generation/TerracottaLayerThemeRuleDefinition.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"

TerracottaLayerThemeFunction getColourFunc(const std::wstring& function);

TerracottaLayerTheme::Pattern getPattern(const std::wstring& pattern)
{
    TerracottaLayerTheme::Pattern result;
    Random random;
    const Direction* directions[4] = {Direction::NORTH, Direction::SOUTH,
                                      Direction::EAST, Direction::WEST};
    const Direction* selected = directions[random.nextInt(4)];
    for (int i = 0; i < 16; ++i)
        result.values[i] = 0;

    if (pattern == L"RANDOM:1")
    {
        result.size = 1;
        result.values[0] = selected;
    }
    else if (pattern == L"RANDOM:2")
    {
        result.size = 2;
        result.values[0] = selected->getClockWise();
        result.values[1] = selected->getOpposite();
        result.values[4] = selected;
        result.values[5] = selected->getCounterClockWise();
    }
    else if (pattern == L"RANDOM:3")
    {
        result.size = 3;
        const Direction* opposite = selected->getOpposite();
        result.values[0] = selected;
        result.values[1] = opposite;
        result.values[2] = selected;
        result.values[5] = opposite;
        result.values[6] = selected;
        result.values[7] = opposite;
        result.values[9] = selected;
        result.values[10] = opposite;
        result.values[11] = selected;
    }
    else
    {
        result.size = 4;
        const Direction* clockwise = selected->getClockWise();
        const Direction* counterClockwise = selected->getCounterClockWise();
        const Direction* opposite = selected->getOpposite();
        result.values[0] = clockwise;
        result.values[1] = selected;
        result.values[2] = clockwise;
        result.values[4] = counterClockwise;
        result.values[5] = opposite;
        result.values[6] = counterClockwise;
        result.values[8] = clockwise;
        result.values[9] = counterClockwise;
        result.values[10] = opposite;
    }
    return result;
}

TerracottaLayerThemeRuleDefinition::TerracottaLayerThemeRuleDefinition()
{
    mPattern.assign(L"RANDOM:2");
    mColourFunction.assign(L"UNIFORM");
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_colour, &mColourFunction);
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_pattern, &mPattern);
}

TerracottaLayerThemeRuleDefinition::~TerracottaLayerThemeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType TerracottaLayerThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_TerracottaTheme;
}

LayerTheme* TerracottaLayerThemeRuleDefinition::Generate() const
{
    TerracottaLayerTheme::Pattern pattern = getPattern(mPattern);
    TerracottaLayerThemeFunction function = getColourFunc(mColourFunction);
    return new TerracottaLayerTheme(pattern, function);
}
