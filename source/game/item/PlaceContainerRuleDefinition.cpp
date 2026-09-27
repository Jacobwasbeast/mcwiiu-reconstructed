#include "game/item/PlaceContainerRuleDefinition.hpp"

#include <boost/pointer_cast.hpp>

#include "game/item/Container.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/BoundingBox.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/StructurePiece.hpp"

PlaceContainerRuleDefinition::PlaceContainerRuleDefinition()
    : XboxStructureActionPlaceBlock(), mChance(1.0f)
{
    mBlock = Blocks::chest;
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_facing, &mAuxValue);
    addFloatAttribute(ConsoleGameRules::eGameRuleAttr_probability, &mChance);
}

PlaceContainerRuleDefinition::~PlaceContainerRuleDefinition()
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mAddItems.begin(); it != mAddItems.end(); ++it)
    {
        delete *it;
    }
    for (std::vector<RandomItemSetRuleDefinition*>::iterator it = mRandomItemSets.begin(); it != mRandomItemSets.end(); ++it)
    {
        delete *it;
    }
}

uint32_t PlaceContainerRuleDefinition::placeContainerInLevel(Level* level, const BlockPos& pos,
                                                              LevelRuleset* ruleset)
{
    boost::shared_ptr<BlockEntity> oldBlockEntity = level->getBlockEntity(pos);
    if (oldBlockEntity)
    {
        level->removeBlockEntity(pos);
        level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
    }

    Random random;
    if (mChance <= random.nextFloat())
    {
        return 0;
    }

    level->setBlock(pos, mBlock->getBlockState(mAuxValue), Block::UPDATE_ALL, false);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(blockEntity);
    if (container)
    {
        for (std::vector<RandomItemSetRuleDefinition*>::iterator it = mRandomItemSets.begin();
             it != mRandomItemSets.end(); ++it)
        {
            (*it)->populateContainer(container, ruleset);
        }

        unsigned int slot = 0;
        for (std::vector<AddItemRuleDefinition*>::iterator it = mAddItems.begin();
             it != mAddItems.end(); ++it)
        {
            if (slot >= container->getContainerSize())
            {
                break;
            }
            (*it)->addItemToContainer(container, static_cast<int>(slot));
            ++slot;
        }
    }
    return 1;
}

uint32_t PlaceContainerRuleDefinition::placeContainerInChunk(StructurePiece* structurePiece, Level* level,
                                                              BoundingBox* box)
{
    BlockPos pos(structurePiece->getWorldX(mEndX, mEndZ),
                 structurePiece->getWorldY(mEndY),
                 structurePiece->getWorldZ(mEndX, mEndZ));
    if (box->isInside(pos))
    {
        return placeContainerInLevel(level, pos, NULL);
    }
    return 0;
}

void PlaceContainerRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mAddItems.begin(); it != mAddItems.end(); ++it)
    {
        children->push_back(*it);
    }
}

GameRuleDefinition* PlaceContainerRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* child = NULL;
    if (type == ConsoleGameRules::eGameRuleType_AddItem)
    {
        AddItemRuleDefinition* addItem = new AddItemRuleDefinition();
        mAddItems.push_back(addItem);
        child = addItem;
    }
    else if (type == ConsoleGameRules::eGameRuleType_RandomItemSet)
    {
        RandomItemSetRuleDefinition* randomItemSet = new RandomItemSetRuleDefinition();
        mRandomItemSets.push_back(randomItemSet);
        child = randomItemSet;
    }
    return child;
}

bool PlaceContainerRuleDefinition::evaluateRule(Level* level, LevelRuleset* ruleset,
                                                 const MiniGameDef& miniGameDef)
{
    (void)miniGameDef;
    BlockPos pos(mEndX, mEndY, mEndZ);
    return placeContainerInLevel(level, pos, ruleset) != 0;
}

ConsoleGameRules::EGameRuleType PlaceContainerRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_PlaceContainer;
}
