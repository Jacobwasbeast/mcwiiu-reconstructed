#include "game/minigame/DegradationSequenceRuleDefinition.hpp"

#include "game/core/DirectionalDegrade.hpp"
#include "game/minigame/DegradationRuleDefinition.hpp"
#include "game/util/RandomDissolveDegrade.hpp"

DegradationSequenceRuleDefinition::DegradationSequenceRuleDefinition()
{
}

DegradationSequenceRuleDefinition::~DegradationSequenceRuleDefinition()
{
    for (std::vector<DegradationRuleDefinition*>::iterator it = mDegradationRules.begin(); it != mDegradationRules.end(); ++it)
    {
        delete *it;
        *it = NULL;
    }
}

ConsoleGameRules::EGameRuleType DegradationSequenceRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_DegradationSequence;
}

void DegradationSequenceRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<DegradationRuleDefinition*>::iterator it = mDegradationRules.begin(); it != mDegradationRules.end(); ++it)
    {
        children->push_back(*it);
    }
}

GameRuleDefinition* DegradationSequenceRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    DegradationRuleDefinition* rule = NULL;

    if (type == ConsoleGameRules::eGameRuleType_RandomDissolveDegrade)
    {
        rule = new RandomDissolveDegrade();
        mDegradationRules.push_back(rule);
    }
    else if (type == ConsoleGameRules::eGameRuleType_DirectionalDegrade)
    {
        rule = new DirectionalDegrade();
        mDegradationRules.push_back(rule);
    }

    return rule;
}
