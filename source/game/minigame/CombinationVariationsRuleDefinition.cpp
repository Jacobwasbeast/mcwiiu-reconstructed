#include "game/minigame/CombinationVariationsRuleDefinition.hpp"

#include <cwchar>

#include "game/minigame/AnyCombinationOfRuleDefinition.hpp"
#include "game/world/generation/LayerGenerationRuleDefinition.hpp"

CombinationVariationsRuleDefinition::CombinationVariationsRuleDefinition(
    LayerGenerationRuleDefinition* parent, AnyCombinationOfRuleDefinition* definition)
    : mParent(parent),
      mDefinition(definition),
      mForWholeLevel(false)
{
}

CombinationVariationsRuleDefinition::~CombinationVariationsRuleDefinition()
{
}

std::vector<AnyCombinationOfRuleDefinition*>* CombinationVariationsRuleDefinition::GetCombinations() const
{
    return const_cast<std::vector<AnyCombinationOfRuleDefinition*>*>(&mCombinations);
}

bool CombinationVariationsRuleDefinition::IsForWholeLevel() const
{
    return mForWholeLevel;
}

void CombinationVariationsRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface,
    unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount);
}

void CombinationVariationsRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(0, name.size(), L"for", wcslen(L"for")) == 0)
    {
        mForWholeLevel = value.compare(0, value.size(), L"WholeLevel", wcslen(L"WholeLevel")) == 0;
        return;
    }

    GameRuleDefinition::addAttribute(name, value);
}

void CombinationVariationsRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    children->insert(children->end(), mCombinations.begin(), mCombinations.end());
}

GameRuleDefinition* CombinationVariationsRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    if (type == ConsoleGameRules::eGameRuleType_AnyCombinationOf)
    {
        AnyCombinationOfRuleDefinition* child =
            new AnyCombinationOfRuleDefinition(mParent, mDefinition);
        mCombinations.push_back(child);
        return child;
    }

    return GameRuleDefinition::addChild(type);
}

ConsoleGameRules::EGameRuleType CombinationVariationsRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_Variations;
}
