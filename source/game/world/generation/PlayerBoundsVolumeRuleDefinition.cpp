#include "game/world/generation/PlayerBoundsVolumeRuleDefinition.hpp"

#include "game/minigame/NamedAreaRuleDefinition.hpp"

PlayerBoundsVolumeRuleDefinition::PlayerBoundsVolumeRuleDefinition()
{
}

PlayerBoundsVolumeRuleDefinition::~PlayerBoundsVolumeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType PlayerBoundsVolumeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(22);
}
