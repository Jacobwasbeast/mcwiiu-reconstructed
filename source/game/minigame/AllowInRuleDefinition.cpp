#include "game/minigame/AllowInRuleDefinition.hpp"

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

std::wstring join(wchar_t separator, const std::vector<std::wstring>& values);
void split(wchar_t separator, const std::wstring& value, std::vector<std::wstring>& values);

AllowInRuleDefinition::AllowInRuleDefinition()
{
}

AllowInRuleDefinition::~AllowInRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType AllowInRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(36);
}

void AllowInRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 2);

    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(56), join(L';', mTargetLayers));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(59), join(L';', mPermissions));
}

void AllowInRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"target") == 0)
    {
        split(L';', value, mTargetLayers);
        return;
    }

    if (name.compare(L"type") == 0)
    {
        split(L';', value, mPermissions);
        return;
    }

    GameRuleDefinition::addAttribute(name, value);
}

const std::vector<std::wstring>& AllowInRuleDefinition::getTargetLayers() const
{
    return mTargetLayers;
}

const std::vector<std::wstring>& AllowInRuleDefinition::getPermissions() const
{
    return mPermissions;
}
