#include "game/minigame/CollisionExceptionGameRuleDefinition.hpp"

#include "game/world/block/Block.hpp"

CollisionExceptionGameRuleDefinition::CollisionExceptionGameRuleDefinition()
    : mBlockId(0)
{
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(12), &mBlockId);
}

CollisionExceptionGameRuleDefinition::~CollisionExceptionGameRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType CollisionExceptionGameRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(72);
}

bool CollisionExceptionGameRuleDefinition::ShouldCollideWith(Block* block)
{
    return Block::getId(block) != mBlockId;
}
