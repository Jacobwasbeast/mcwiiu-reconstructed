#include "game/core/GameRules_BoolAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"

#include <string>

std::string formatstr(const char* format, ...);

GameRules_BoolAttribute::GameRules_BoolAttribute(bool* value)
    : mValue(value)
{
}

GameRules_BoolAttribute::~GameRules_BoolAttribute()
{
}

void GameRules_BoolAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = (value.compare(L"true") == 0);
}

void GameRules_BoolAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, *mValue ? L"true" : L"false");
}

void GameRules_BoolAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%s\"", IGameRuleAttribute::attributeName(attribute), *mValue ? "true" : "false"));
}
