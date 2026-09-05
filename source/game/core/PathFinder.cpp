#include "game/core/PathFinder.hpp"

#include <cmath>

#include "game/core/Path.hpp"
#include "game/entity/ai/NodeEvaluator.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

PathFinder::PathFinder(NodeEvaluator* nodeEvaluator)
    : BinaryHeap(), mNeighbors(new arrayWithLength<Node*>(32, true)), mNodeEvaluator(nodeEvaluator)
{
}

PathFinder::~PathFinder()
{
    delete mNeighbors;
}

Path* PathFinder::reconstruct_path(Node* start, Node* end)
{
    unsigned int length = 1;
    Node* node = end;
    while (node->cameFrom != NULL)
    {
        ++length;
        node = node->cameFrom;
    }

    arrayWithLength<Node*> nodes;
    nodes.data = new Node*[length];
    nodes.length = length;
    nodes.data[length - 1] = end;
    node = end;
    for (unsigned int i = length - 1; node->cameFrom != NULL; --i)
    {
        node = node->cameFrom;
        nodes.data[i - 1] = node;
    }

    Path* path = new Path(nodes);
    delete[] nodes.data;
    return path;
}

Path* PathFinder::findPath(Node* start, Node* end, float maxDistance)
{
    start->g = 0.0f;
    start->h = (float)start->distanceManhattan(end);
    start->f = start->h;
    clear();
    insert(start);

    Node* best = start;
    int iterations = 0;
    while (!isEmpty() && iterations++ < 200)
    {
        Node* current = pop();
        if (current->equals(end))
        {
            best = end;
            break;
        }
        if (current->distanceManhattan(end) < best->distanceManhattan(end))
        {
            best = current;
        }
        current->closed = true;
        int count = mNodeEvaluator->getNeighbors(mNeighbors, current, end, maxDistance);
        for (int i = 0; i < count; ++i)
        {
            Node* neighbor = mNeighbors->data[i];
            float distance = (float)current->distanceManhattan(neighbor);
            float walkedDistance = current->walkedDistance + distance;
            float cost = distance + neighbor->costMalus;
            float totalCost = current->g + cost;
            if (walkedDistance >= maxDistance || (neighbor->inOpenSet() && neighbor->g <= totalCost))
            {
                continue;
            }
            neighbor->cameFrom = current;
            neighbor->g = totalCost;
            neighbor->h = (float)neighbor->distanceManhattan(end) + neighbor->costMalus;
            if (!neighbor->inOpenSet())
            {
                neighbor->f = neighbor->g + neighbor->h;
                insert(neighbor);
            }
            else
            {
                changeCost(neighbor, neighbor->f);
            }
        }
    }

    if (best == start)
    {
        return NULL;
    }
    return reconstruct_path(start, best);
}

Path* PathFinder::findPath(LevelSource* levelSource, Mob* mob, double x, double y, double z, float maxDistance)
{
    clear();
    mNodeEvaluator->prepare(levelSource, mob);
    Node* start = mNodeEvaluator->getStart();
    Entity* entity = (Entity*)mob;
    BlockPos mobPos(entity->mX, entity->mY, entity->mZ);
    Block* block = levelSource->getBlock(mobPos);
    if (block->instanceof(EBLOCK_TYPE_RAIL))
    {
        int mobX = (int)floor(entity->mX + 0.5);
        int mobZ = (int)floor(entity->mZ + 0.5);
        if ((double)mobX == floor(entity->mX)) --mobX;
        if ((double)mobZ == floor(entity->mZ)) --mobZ;
        Node* adjusted = mNodeEvaluator->getNode(mobX, (int)start->y, mobZ);
        if (adjusted != NULL)
        {
            start = adjusted;
        }
    }
    Node* goal = mNodeEvaluator->getGoal(x, y, z);
    Path* path = findPath(start, goal, maxDistance);
    mNodeEvaluator->done();
    return path;
}

Path* PathFinder::findPath(LevelSource* levelSource, Mob* mob, const BlockPos& pos, float maxDistance)
{
    return findPath(levelSource, mob, (double)pos.x + 0.5, (double)pos.y + 0.5, (double)pos.z + 0.5, maxDistance);
}

Path* PathFinder::findPath(LevelSource* levelSource, Mob* mob, Entity* entity, float maxDistance)
{
    clear();
    mNodeEvaluator->prepare(levelSource, mob);
    Node* start = mNodeEvaluator->getStart();
    BlockPos entityPos(entity->mX, entity->mY, entity->mZ);
    Block* block = levelSource->getBlock(entityPos);
    if (block->instanceof(EBLOCK_TYPE_RAIL))
    {
        int entityX = (int)floor(entity->mX + 0.5);
        int entityZ = (int)floor(entity->mZ + 0.5);
        if ((double)entityX == floor(entity->mX)) --entityX;
        if ((double)entityZ == floor(entity->mZ)) --entityZ;
        Node* adjusted = mNodeEvaluator->getNode(entityX, (int)start->y, entityZ);
        if (adjusted != NULL)
        {
            start = adjusted;
        }
    }
    Node* goal = mNodeEvaluator->getGoal(entity->mX, entity->mY, entity->mZ);
    Path* path = findPath(start, goal, maxDistance);
    mNodeEvaluator->done();
    return path;
}

Path* PathFinder::findDirectPath(LevelSource* levelSource, Mob* mob, const BlockPos& pos, float maxDistance)
{
    clear();
    mNodeEvaluator->prepare(levelSource, mob);
    Node* start = mNodeEvaluator->getStart();
    Node* goal = mNodeEvaluator->getDirectGoal(pos);
    Path* path = findPath(start, goal, maxDistance);
    mNodeEvaluator->done();
    return path;
}
