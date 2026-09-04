#include "game/world/block/BlockLayerRuleDefinition.hpp"

#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/block/BlockPos.hpp"

BlockLayerRuleDefinition::BlockLayerRuleDefinition()
{
    mBlockId = 0;

    GameRules_IntAttribute* blockId = addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(12), &mBlockId);
    blockId->setMin(0);
    blockId->setMax(255);
}

BlockLayerRuleDefinition::~BlockLayerRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType BlockLayerRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(24);
}

bool BlockLayerRuleDefinition::containsBlock(int blockId, const BlockPos& pos)
{
    return mBlockId == blockId && NamedAreaRuleDefinition::containsBlock(blockId, pos);
}
