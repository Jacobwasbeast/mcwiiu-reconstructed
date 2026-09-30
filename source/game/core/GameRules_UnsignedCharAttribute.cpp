#include "game/core/GameRules_UnsignedCharAttribute.hpp"

#include "game/core/IGameRuleAttribute.hpp"
#include "game/util/StringUtils.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringConversion.hpp"

#include <string>


template<typename T>
T fromString(const std::wstring& text);

GameRules_UnsignedCharAttribute::GameRules_UnsignedCharAttribute(unsigned char* value)
    : mValue(value)
{
}

GameRules_UnsignedCharAttribute::~GameRules_UnsignedCharAttribute()
{
}

void GameRules_UnsignedCharAttribute::setValue(const std::wstring& value)
{
    mHasValue = true;

    *mValue = fromString<int>(value);
}

void GameRules_UnsignedCharAttribute::save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface)
{
    saveInterface->saveAttribute(attribute, _toString(*mValue));
}

void GameRules_UnsignedCharAttribute::writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    xml.append(formatstr(" %ls=\"%d\"", IGameRuleAttribute::attributeName(attribute), *mValue));
}
