#include "game/core/GameRules_StringAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"

#include <string>

std::string formatstr(const char* format, ...);

GameRules_StringAttribute::GameRules_StringAttribute(std::wstring* value)
    : mValue(value)
{
}

GameRules_StringAttribute::~GameRules_StringAttribute()
{
}

void GameRules_StringAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    mValue->assign(value, 0, std::wstring::npos);
}

void GameRules_StringAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, *mValue);
}

void GameRules_StringAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%ls\"", IGameRuleAttribute::attributeName(attribute), mValue->c_str()));
}
