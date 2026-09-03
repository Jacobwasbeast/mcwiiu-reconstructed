#include "game/minigame/RandomizeSizeRuleDefinition.hpp"

#include "game/util/RandomizeSize.hpp"

RandomizeSizeRuleDefinition::RandomizeSizeRuleDefinition()
    : mAmount(-1.0f)
    , mChild(NULL)
{
}

RandomizeSizeRuleDefinition::~RandomizeSizeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType RandomizeSizeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(46);
}

LayerSize* RandomizeSizeRuleDefinition::Generate() const
{
    if (mChild == NULL)
    {
        return NULL;
    }

    return new RandomizeSize(mChild->Generate(), mAmount);
}
