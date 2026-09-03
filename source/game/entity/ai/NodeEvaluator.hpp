#ifndef GAME_ENTITY_AI_NODEEVALUATOR_HPP
#define GAME_ENTITY_AI_NODEEVALUATOR_HPP

#include <boost/unordered_map.hpp>

#include "game/core/Node.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class BlockPathTypes;
class BlockPos;
class EntityTickCache;
class LevelSource;
class Mob;
class BlockState;

class NodeEvaluator
{
public:
    NodeEvaluator();
    virtual ~NodeEvaluator();

    virtual void prepare(LevelSource* levelSource, Mob* mob);
    virtual void done();
    virtual Node* getNode(int x, int y, int z);
    virtual Node* getStart() = 0;
    virtual Node* getGoal(double x, double y, double z) = 0;
    virtual Node* getDirectGoal(const BlockPos& pos);
    virtual int getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost) = 0;
    virtual int getBlockPathTypes(LevelSource* levelSource, int x, int y, int z, int sizeX, int sizeY, int sizeZ, bool canOpenDoors, bool canPassDoors, int& nodeType, const BlockPathTypes* pathTypes) = 0;

    void setCanPassDoors(bool value);
    void setCanOpenDoors(bool value);
    void setCanFloat(bool value);
    bool isCanPassDoors();
    bool isCanOpenDoors();
    bool isCanFloat();
    void setSinkInWaterWalk(bool value);
    bool isCanSinkInWaterWalk();
    void getBlockState(LevelSource* levelSource, const BlockPos& pos);
    void getBlockState(EntityTickCache& tickCache, LevelSource* levelSource, const BlockPos& pos);
    unsigned int getNodesCount();

protected:
    LevelSource* mLevelSource;
    Mob* mMob;
    boost::unordered_map<int, Node*> mNodeMap;
    boost::unordered_map<int, const BlockState*> mBlockStateMap;
    boost::unordered_map<int, int> mBlockPathTypeMap;
    int mEntityWidth;
    int mEntityHeight;
    int mEntityY;
    bool mCanPassDoors;
    bool mCanOpenDoors;
    bool mCanFloat;
    bool mCanSinkInWaterWalk;
};

#endif
