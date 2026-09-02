#include "game/core/Node.hpp"

#include <cstdlib>
#include <vector>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/BlockPathTypes.hpp"

static const unsigned int NODE_POOL_SIZE = 1024;

static unsigned int sNodesUsed = 0;
static bool sUsePool = true;
static Node sNodePool[NODE_POOL_SIZE];
static std::vector<Node*> sOverflowNodes;

Node::Node()
    : x(0),
      y(0),
      z(0),
      hash(0)
{
    _init();
}

Node::Node(int x, int y, int z)
    : x(x),
      y(y),
      z(z),
      hash(createHash(x, y, z))
{
    _init();
}

Node::Node(const Node& other)
    : x(other.x),
      y(other.y),
      z(other.z),
      hash(other.hash),
      heapIdx(other.heapIdx),
      g(other.g),
      h(other.h),
      f(other.f),
      cameFrom(other.cameFrom),
      closed(other.closed),
      walkedDistance(other.walkedDistance),
      moveCost(other.moveCost),
      costMalus(other.costMalus),
      type(other.type),
      reached(other.reached),
      searchId(other.searchId)
{
}

void Node::ResetPool()
{
    sNodesUsed = 0;
}

Node* Node::GetNodeFromPool(int x, int y, int z)
{
    Node* node = NULL;

    if (!sUsePool)
    {
        node = new Node(x, y, z);
    }
    else if (sNodesUsed < NODE_POOL_SIZE)
    {
        node = &sNodePool[sNodesUsed];
        node->x = x;
        node->y = y;
        node->z = z;
        node->hash = createHash(x, y, z);
        node->_init();
        ++sNodesUsed;
    }
    else if (sNodesUsed - NODE_POOL_SIZE < sOverflowNodes.size())
    {
        node = sOverflowNodes[sNodesUsed - NODE_POOL_SIZE];
        node->x = x;
        node->y = y;
        node->z = z;
        node->hash = createHash(x, y, z);
        node->_init();
        ++sNodesUsed;
    }
    else
    {
        node = new Node(x, y, z);
        sOverflowNodes.push_back(node);
        node->x = x;
        node->y = y;
        node->z = z;
        node->hash = createHash(x, y, z);
        node->_init();
        ++sNodesUsed;
    }

    return node;
}

int Node::createHash(int x, int y, int z)
{
    return (y & 255) | ((x & 32767) << 8) | ((z & 32767) << 24) | (x < 0 ? (int)0x80000000 : 0)
        | (z < 0 ? 32768 : 0);
}

Node* Node::createFromStream(DataInputStream* stream)
{
    int x = stream->readInt();
    int y = stream->readInt();
    int z = stream->readInt();

    Node* node = GetNodeFromPool(x, y, z);

    node->walkedDistance = stream->readFloat();
    node->moveCost = stream->readFloat();
    node->costMalus = stream->readFloat();
    node->closed = stream->readBoolean();
    node->type = BlockPathTypes::VALUES[stream->readInt()];
    node->f = stream->readFloat();

    return node;
}

void Node::_init()
{
    heapIdx = -1;
    cameFrom = NULL;
    closed = false;
    walkedDistance = 0.0F;
    moveCost = 0.0F;
    costMalus = 0.0F;
    type = BlockPathTypes::BLOCKED;
    reached = false;
    searchId = 0;
}

Node* Node::cloneMove(int x, int y, int z)
{
    Node* node = GetNodeFromPool(x, y, z);

    node->heapIdx = heapIdx;
    node->g = g;
    node->h = h;
    node->f = f;
    node->cameFrom = cameFrom;
    node->closed = closed;
    node->walkedDistance = walkedDistance;
    node->moveCost = moveCost;
    node->costMalus = costMalus;
    node->type = type;

    return node;
}

float Node::distanceTo(Node* node)
{
    float dx = (float)(node->x - x);
    float dy = (float)(node->y - y);
    float dz = (float)(node->z - z);

    return Mth::sqrt(dx * dx + dy * dy + dz * dz);
}

float Node::distanceManhattan(Node* node)
{
    float dx = (float)std::abs(node->x - x);
    float dy = (float)std::abs(node->y - y);
    float dz = (float)std::abs(node->z - z);

    return dx + dy + dz;
}

bool Node::equals(Node* node)
{
    return hash == node->hash && x == node->x && y == node->y && z == node->z;
}

bool Node::inOpenSet()
{
    return heapIdx >= 0;
}

void Node::writeToStream(DataOutputStream* stream)
{
    stream->writeInt(x);
    stream->writeInt(y);
    stream->writeInt(z);
    stream->writeFloat(walkedDistance);
    stream->writeFloat(moveCost);
    stream->writeFloat(costMalus);
    stream->writeBoolean(closed);
    stream->writeInt(type->ordinal());
    stream->writeFloat(f);
}
