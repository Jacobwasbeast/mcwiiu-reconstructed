#include "game/item/DistributeItemsRuleDefinition.hpp"

#include <boost/pointer_cast.hpp>

#include "game/item/Container.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/WorldPositionRuleDefinition.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"

class DistributeItemsWorldPositionAccess : public WorldPositionRuleDefinition
{
public:
    static BlockPos getPosition(const WorldPositionRuleDefinition* position)
    {
        const DistributeItemsWorldPositionAccess* access = static_cast<const DistributeItemsWorldPositionAccess*>(position);
        return BlockPos(access->mX, access->mY, access->mZ);
    }
};

DistributeItemsRuleDefinition::DistributeItemsRuleDefinition()
    : GameRuleDefinition()
{
    mLootGroup.assign(L"rule");
    addStringAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(49), &mLootGroup);
}

DistributeItemsRuleDefinition::~DistributeItemsRuleDefinition()
{
    for (std::vector<WorldPositionRuleDefinition*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
    {
        delete *it;
    }
}

void DistributeItemsRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<WorldPositionRuleDefinition*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
    {
        children->push_back(*it);
    }
}

ConsoleGameRules::EGameRuleType DistributeItemsRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(14);
}

GameRuleDefinition* DistributeItemsRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    WorldPositionRuleDefinition* child = NULL;
    if (type == static_cast<ConsoleGameRules::EGameRuleType>(15))
    {
        child = new WorldPositionRuleDefinition();
        mPositions.push_back(child);
    }
    return child;
}

void DistributeItemsRuleDefinition::getContainers(std::vector<boost::shared_ptr<Container> >* containers, Level* level, const std::vector<WorldPositionRuleDefinition*>& positions)
{
    for (std::vector<WorldPositionRuleDefinition*>::const_iterator it = positions.begin(); it != positions.end(); ++it)
    {
        BlockPos position = DistributeItemsWorldPositionAccess::getPosition(*it);
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(position);
        boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(blockEntity);
        if (container)
        {
            containers->push_back(container);
        }
    }
}

bool DistributeItemsRuleDefinition::evaluateRule(Level* level, LevelRuleset* ruleset, const MiniGameDef& miniGameDef)
{
    (void)ruleset;
    std::vector<boost::shared_ptr<Container> > containers;
    getContainers(&containers, level, mPositions);
    return miniGameDef.PopulateContainers(&containers, mLootGroup) != 0;
}

std::wstring DistributeItemsRuleDefinition::getId() const
{
    return mLootGroup;
}
