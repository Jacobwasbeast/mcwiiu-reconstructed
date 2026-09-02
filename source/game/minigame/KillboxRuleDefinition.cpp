#include "game/minigame/KillboxRuleDefinition.hpp"

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/util/AABB.hpp"

KillboxRuleDefinition::KillboxRuleDefinition(AABB* legacyArea)
    : mDeathMessageIDS(0)
{
    if (legacyArea == NULL)
    {
        addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(90), &mDeathMessageIDS);
    }
    else
    {
        ConvertFromLegacy(legacyArea);
    }
}

KillboxRuleDefinition::~KillboxRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType KillboxRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(23);
}

void KillboxRuleDefinition::ConvertFromLegacy(AABB* legacyArea)
{
    updateWithPoints(static_cast<int>(legacyArea->mMinX),
                     static_cast<int>(legacyArea->mMinY),
                     static_cast<int>(legacyArea->mMinZ),
                     static_cast<int>(legacyArea->mMaxX),
                     static_cast<int>(legacyArea->mMaxY),
                     static_cast<int>(legacyArea->mMaxZ));
}

void KillboxRuleDefinition::GetDeathMessageIDS(int& deathMessageIDS, std::wstring& deathMessage)
{
    if (mDeathMessageIDS != 0)
    {
        deathMessageIDS = mDeathMessageIDS;
        deathMessage = L"";
        return;
    }

    if (mDescriptionName != L"")
    {
        deathMessageIDS = 0;
        deathMessage = mDescriptionName;
        return;
    }

    deathMessageIDS = 0x6F2C7352;
    deathMessage = L"";
}

void KillboxRuleDefinition::WriteAttributesAsXML(std::string& xml)
{
    if (!mName.empty())
    {
        WriteAttributeAsXML(static_cast<ConsoleGameRules::EGameRuleAttr>(9), xml);
    }

    WriteAttributeAsXML(static_cast<ConsoleGameRules::EGameRuleAttr>(90), xml);

    NamedAreaRuleDefinition::WriteAttributesAsXML(xml);
}
