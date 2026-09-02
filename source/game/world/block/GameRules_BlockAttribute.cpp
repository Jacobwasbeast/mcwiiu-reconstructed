#include "game/world/block/GameRules_BlockAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/world/block/Block.hpp"

#include <string>

std::string formatstr(const char* format, ...);

template<typename T>
std::wstring toString(T value);

template<typename T>
T fromString(const std::wstring& text);

GameRules_BlockAttribute::GameRules_BlockAttribute(Block** value)
    : mValue(value)
{
}

GameRules_BlockAttribute::~GameRules_BlockAttribute()
{
}

void GameRules_BlockAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = Block::byId(fromString<int>(value));
}

void GameRules_BlockAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, toString(*mValue));
}

void GameRules_BlockAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%ls\"", IGameRuleAttribute::attributeName(attribute), toString(*mValue).c_str()));
}
