#include "game/world/block/GenericMultiBlockThemeRuleDefinition.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"

GenericMultiBlockThemeRuleDefinition::~GenericMultiBlockThemeRuleDefinition()
{
}

GameRuleDefinition* GenericMultiBlockThemeRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    if (type == ConsoleGameRules::eGameRuleType_BlockDef)
    {
        BlockRuleDefinition* blockRule = new BlockRuleDefinition();
        mBlocks.push_back(blockRule);
        return blockRule;
    }

    return GameRuleDefinition::addChild(type);
}

void GenericMultiBlockThemeRuleDefinition::fillStack(std::vector<const BlockState*>& stack) const
{
    for (std::vector<BlockRuleDefinition*>::const_iterator block = mBlocks.begin(); block != mBlocks.end(); ++block)
    {
        stack.push_back((*block)->getBlockState());
    }
}

GenericMultiBlockThemeRuleDefinition::BlockRuleDefinition::BlockRuleDefinition()
    : mBlockState(NULL),
      mBlockAux(-1)
{
    addBlockStateAttribute(ConsoleGameRules::eGameRuleAttr_blockId, &mBlockState);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_blockAux, &mBlockAux);
}

GenericMultiBlockThemeRuleDefinition::BlockRuleDefinition::~BlockRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType GenericMultiBlockThemeRuleDefinition::BlockRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_BlockDef;
}

const BlockState* GenericMultiBlockThemeRuleDefinition::BlockRuleDefinition::getBlockState() const
{
    if (mBlockAux > 0)
    {
        return mBlockState->getBlock()->getBlockState(mBlockAux);
    }

    return mBlockState;
}

int GenericMultiBlockThemeRuleDefinition::BlockRuleDefinition::getBlockId() const
{
    return Block::getId(mBlockState->getBlock());
}

int GenericMultiBlockThemeRuleDefinition::BlockRuleDefinition::getBlockAux() const
{
    if (mBlockAux > 0)
    {
        return mBlockAux;
    }

    if (mBlockState != NULL)
    {
        return mBlockState->getBlock()->convertBlockStateToLegacyData(mBlockState);
    }

    return -1;
}
