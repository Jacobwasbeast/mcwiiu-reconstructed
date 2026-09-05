#include "game/minigame/DegradationRuleDefinition.hpp"

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

template<typename T>
std::wstring _toString(T value);

DegradationRuleDefinition::DegradationRuleDefinition()
    : mTargetLayer(L"_UNINTIALISED_"),
      mSpeed(2)
{
    addStringAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(56), &mTargetLayer);
}

DegradationRuleDefinition::~DegradationRuleDefinition()
{
}

void DegradationRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 1);

    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(57), _toString(mSpeed));
}

void DegradationRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"speed") == 0)
    {
        if (value.compare(L"Uniform") == 0)
        {
            mSpeed = 2;
        }

        return;
    }

    GameRuleDefinition::addAttribute(name, value);
}

void DegradationRuleDefinition::ClearCaches()
{
}

const std::wstring& DegradationRuleDefinition::getTargetLayer() const
{
    return mTargetLayer;
}
