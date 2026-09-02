#ifndef GAME_MINIGAME_COMBINATIONDEFINITIONRULEDEFINITION_HPP
#define GAME_MINIGAME_COMBINATIONDEFINITIONRULEDEFINITION_HPP

#include "game/data/GameRuleSaveInterface.hpp"

#include <string>
#include "game/minigame/AnyCombinationOfRuleDefinition.hpp"

class AnyCombinationOfRuleDefinition;
class LayerGenerationRuleDefinition;

class CombinationDefinitionRuleDefinition : public AnyCombinationOfRuleDefinition
{
public:
    CombinationDefinitionRuleDefinition(LayerGenerationRuleDefinition* parent);

    virtual ~CombinationDefinitionRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();

    const std::wstring& GetName() const;

private:
    std::wstring mName;
};

#endif
