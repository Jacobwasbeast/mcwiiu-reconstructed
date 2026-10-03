#include "game/core/GameRules_HexAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/core/StringConversion.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/core/StringFormat.hpp"

#include <string>

template<typename T>
std::wstring toString(T value);

template<typename T>
T fromHEXString(const std::wstring& text)
{
    return _fromHEXString<T>(text);
}

template int fromHEXString<int>(const std::wstring&);

GameRules_HexAttribute::GameRules_HexAttribute(int* value)
    : mValue(value)
{
}

GameRules_HexAttribute::~GameRules_HexAttribute()
{
}

void GameRules_HexAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = fromHEXString<int>(value);
}

void GameRules_HexAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, toString(*mValue));
}

void GameRules_HexAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%x\"", gRuleAttributeNames[attribute], *mValue));
}
