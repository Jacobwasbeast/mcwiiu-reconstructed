#include "game/core/GameRules_IntAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/core/StringFormat.hpp"

#include <algorithm>
#include <string>

template<typename T>
std::wstring toString(T value);

template<typename T>
T fromString(const std::wstring& text);

GameRules_IntAttribute::GameRules_IntAttribute(int* value)
    : mValue(value),
      mMin(0),
      mMax(0)
{
    mHasMin = false;
    mHasMax = false;
}

GameRules_IntAttribute::~GameRules_IntAttribute()
{
}

void GameRules_IntAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = fromString<int>(value);

    if (mHasMin)
    {
        *mValue = std::max(*mValue, mMin);
    }

    if (mHasMax)
    {
        *mValue = std::min(*mValue, mMax);
    }
}

void GameRules_IntAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, toString(*mValue));
}

void GameRules_IntAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%d\"", IGameRuleAttribute::attributeName(attribute), *mValue));
}

void GameRules_IntAttribute::setMin(int min)
{
    mMin = min;
    mHasMin = true;
}

void GameRules_IntAttribute::setMax(int max)
{
    mMax = max;
    mHasMax = true;
}
