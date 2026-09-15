#include "game/entity/ai/NodeEvaluator.hpp"

#include "game/entity/mob/EntityTickCache.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"

NodeEvaluator::NodeEvaluator()
{
    mMob = 0;
    mCanPassDoors = false;
    mCanOpenDoors = false;
    mCanFloat = false;
    mCanSinkInWaterWalk = false;
}

NodeEvaluator::~NodeEvaluator()
{
    mNodeMap.clear();
    mBlockStateMap.clear();
    mBlockPathTypeMap.clear();
    mLevelSource = 0;
    mMob = 0;
}

void NodeEvaluator::prepare(LevelSource* levelSource, Mob* mob)
{
    mLevelSource = levelSource;
    mMob = mob;
    mNodeMap.clear();
    mBlockStateMap.clear();
    mBlockPathTypeMap.clear();
    Node::ResetPool();
    float width = mob->mBbWidth + 1.0f;
    float height = mob->mBbHeight + 1.0f;
    mEntityWidth = static_cast<int>(width);
    if (static_cast<float>(mEntityWidth) > width)
        --mEntityWidth;
    mEntityHeight = static_cast<int>(height);
    if (static_cast<float>(mEntityHeight) > height)
        --mEntityHeight;
    mEntityY = static_cast<int>(width);
    if (width < static_cast<float>(mEntityY))
        --mEntityY;
}

void NodeEvaluator::done()
{
}

Node* NodeEvaluator::getNode(int x, int y, int z)
{
    unsigned int hash = Node::createHash(x, y, z);
    boost::unordered_map<int, Node*>::iterator it = mNodeMap.find(static_cast<int>(hash));
    if (it != mNodeMap.end())
        return it->second;
    Node* node = Node::GetNodeFromPool(x, y, z);
    mNodeMap.insert(std::pair<int, Node*>(static_cast<int>(hash), node));
    return node;
}

Node* NodeEvaluator::getDirectGoal(const BlockPos& pos)
{
    (void)pos;
    return 0;
}

void NodeEvaluator::setCanPassDoors(bool value)
{
    mCanPassDoors = value;
}

void NodeEvaluator::setCanOpenDoors(bool value)
{
    mCanOpenDoors = value;
}

void NodeEvaluator::setCanFloat(bool value)
{
    mCanFloat = value;
}

bool NodeEvaluator::isCanPassDoors()
{
    return mCanPassDoors;
}

bool NodeEvaluator::isCanOpenDoors()
{
    return mCanOpenDoors;
}

bool NodeEvaluator::isCanFloat()
{
    return mCanFloat;
}

void NodeEvaluator::setSinkInWaterWalk(bool value)
{
    mCanSinkInWaterWalk = value;
}

bool NodeEvaluator::isCanSinkInWaterWalk()
{
    return mCanSinkInWaterWalk;
}

const BlockState* NodeEvaluator::getBlockState(LevelSource* levelSource, const BlockPos& pos)
{
    return levelSource->getBlockState(pos);
}

const BlockState* NodeEvaluator::getBlockState(EntityTickCache& tickCache, LevelSource* levelSource,
                                               const BlockPos& pos)
{
    return levelSource->getBlockState(tickCache, pos);
}
