#include "game/core/GameRules_DoubleAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/util/StringUtils.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringConversion.hpp"

#include <algorithm>
#include <string>


template<typename T>
T fromString(const std::wstring& text);

GameRules_DoubleAttribute::GameRules_DoubleAttribute(double* value)
    : mValue(value)
{
    mHasMin = false;
    mHasMax = false;
    mMin = 0.0;
    mMax = 0.0;
}

GameRules_DoubleAttribute::~GameRules_DoubleAttribute()
{
}

void GameRules_DoubleAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = fromString<double>(value);

    if (mHasMin)
    {
        *mValue = std::max(*mValue, mMin);
    }

    if (mHasMax)
    {
        *mValue = std::min(*mValue, mMax);
    }
}

void GameRules_DoubleAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, _toString(*mValue));
}

void GameRules_DoubleAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%.1f\"", IGameRuleAttribute::attributeName(attribute), *mValue));
}

void GameRules_DoubleAttribute::setMin(double min)
{
    mMin = min;
    mHasMin = true;
}

void GameRules_DoubleAttribute::setMax(double max)
{
    mMax = max;
    mHasMax = true;
}
