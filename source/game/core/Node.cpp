#include "game/core/Node.hpp"

#include <cmath>
#include <sstream>
#include <string>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/world/block/BlockPathTypes.hpp"

unsigned int Node::poolSize = 0;
bool Node::usePool = false;
Node Node::pool[1024];
std::vector<Node*> Node::allocated;

void Node::ResetPool()
{
    poolSize = 0;
}

unsigned int Node::createHash(int x, int y, int z)
{
    unsigned int result = 0;
    if (x < 0)
        result = 0x80000000u;
    if (z < 0)
        result |= 0x8000u;
    return (static_cast<unsigned int>(z) << 24) | ((static_cast<unsigned int>(x) & 0x7fffu) << 8) |
           (static_cast<unsigned int>(y) & 0xffu) | result;
}

void Node::_init()
{
    costMalus = 4.5f;
    type = 0;
    field3c = 0;
    heapIdx = -1;
    closed = false;
    field2c = 4.5f;
    inOpen = false;
    field20 = 0;
    field30 = 4.5f;
}

Node::Node(int xValue, int yValue, int zValue) : x(xValue), y(yValue), z(zValue), hash(createHash(xValue, yValue, zValue))
{
    _init();
}

Node::Node() : x(0), y(0), z(0), hash(0)
{
    _init();
}

Node::Node(const Node& node)
    : x(node.x), y(node.y), z(node.z), hash(node.hash), heapIdx(node.heapIdx), g(node.g), h(node.h), f(node.f),
      field20(node.field20), closed(node.closed), costMalus(node.costMalus), field2c(node.field2c), field30(node.field30),
      type(node.type), inOpen(node.inOpen), field3c(node.field3c)
{
}

Node* Node::GetNodeFromPool(int xValue, int yValue, int zValue)
{
    Node* node;
    if (!usePool)
        node = new Node(xValue, yValue, zValue);
    else if (poolSize < 1024)
    {
        node = &pool[poolSize++];
        node->x = xValue;
        node->y = yValue;
        node->z = zValue;
        node->hash = createHash(xValue, yValue, zValue);
        node->_init();
    }
    else
    {
        unsigned int index = poolSize - 1024;
        if (index < allocated.size())
        {
            node = allocated[index];
            node->x = xValue;
            node->y = yValue;
            node->z = zValue;
            node->hash = createHash(xValue, yValue, zValue);
            node->_init();
            ++poolSize;
        }
        else
        {
            node = new Node(xValue, yValue, zValue);
            allocated.push_back(node);
            ++poolSize;
        }
    }
    return node;
}

Node* Node::cloneMove(int xValue, int yValue, int zValue)
{
    Node* node = GetNodeFromPool(xValue, yValue, zValue);
    node->g = g;
    node->h = h;
    node->f = f;
    node->field20 = field20;
    node->closed = closed;
    node->costMalus = costMalus;
    node->field2c = field2c;
    node->field30 = field30;
    node->type = type;
    node->heapIdx = heapIdx;
    return node;
}

double Node::distanceTo(Node* node)
{
    float dx = static_cast<float>(node->x - x);
    float dy = static_cast<float>(node->y - y);
    float dz = static_cast<float>(node->z - z);
    return static_cast<double>(std::sqrt(dx * dx + dy * dy + dz * dz));
}

double Node::distanceToSqr(Node* node)
{
    double dx = static_cast<double>(node->x - x);
    double dy = static_cast<double>(node->y - y);
    double dz = static_cast<double>(node->z - z);
    return dx * dx + dy * dy + dz * dz;
}

double Node::distanceManhattan(Node* node)
{
    return static_cast<double>(std::abs(node->x - x) + std::abs(node->y - y) + std::abs(node->z - z));
}

double Node::distanceManhattan(const BlockPos&)
{
    return 0.0;
}

bool Node::equals(Node* node)
{
    return node != 0 && hash == node->hash && x == node->x && y == node->y && z == node->z;
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
    stream->writeFloat(costMalus);
    stream->writeFloat(field2c);
    stream->writeFloat(field30);
    stream->writeBoolean(closed);
    stream->writeInt(type != 0 ? type->ordinal() : 0);
    stream->writeFloat(f);
}

Node* Node::createFromStream(DataInputStream* stream)
{
    Node* node = GetNodeFromPool(stream->readInt(), stream->readInt(), stream->readInt());
    node->costMalus = stream->readFloat();
    node->field2c = stream->readFloat();
    node->field30 = stream->readFloat();
    node->closed = stream->readBoolean();
    node->type = BlockPathTypes::VALUES[stream->readInt()];
    node->f = stream->readFloat();
    return node;
}

const BlockPathTypes* Node::GetType()
{
    return type;
}

int Node::hashCode()
{
    return static_cast<int>(hash);
}

std::wstring Node::toString()
{
    std::wstringstream stream;
    stream << L"Node{" << x << L"," << y << L"," << z << L"}";
    return stream.str();
}
