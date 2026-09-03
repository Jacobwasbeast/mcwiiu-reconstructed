#ifndef GAME_MINIGAME_RANDOMIZESIZERULEDEFINITION_HPP
#define GAME_MINIGAME_RANDOMIZESIZERULEDEFINITION_HPP

#include "game/core/LayerSize.hpp"
#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"

class RandomizeSizeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerSize*>
{
public:
    RandomizeSizeRuleDefinition();
    virtual ~RandomizeSizeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();

    virtual LayerSize* Generate() const;

private:
    float mAmount;
    LayerAttributeChildRuleDefinition<LayerSize*>* mChild;
};

#endif
