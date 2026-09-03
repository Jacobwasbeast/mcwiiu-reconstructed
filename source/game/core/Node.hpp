#ifndef GAME_CORE_NODE_HPP
#define GAME_CORE_NODE_HPP

#include <vector>
#include <string>

class BlockPathTypes;
class BlockPos;
class DataInputStream;
class DataOutputStream;

class Node
{
public:
    Node();
    Node(int x, int y, int z);
    Node(const Node& node);

    static void ResetPool();
    static unsigned int createHash(int x, int y, int z);
    void _init();
    static Node* GetNodeFromPool(int x, int y, int z);
    Node* cloneMove(int x, int y, int z);
    double distanceTo(Node* node);
    double distanceToSqr(Node* node);
    double distanceManhattan(Node* node);
    double distanceManhattan(const BlockPos& pos);
    bool equals(Node* node);
    bool inOpenSet();
    void writeToStream(DataOutputStream* stream);
    static Node* createFromStream(DataInputStream* stream);
    const BlockPathTypes* GetType();
    int hashCode();
    std::wstring toString();

private:
    int x;
    int y;
    int z;
    unsigned int hash;
    int heapIdx;
    float g;
    float h;
    float f;
    int field20;
    bool closed;
    float costMalus;
    float field2c;
    float field30;
    const BlockPathTypes* type;
    bool inOpen;
    int field3c;

    static unsigned int poolSize;
    static bool usePool;
    static Node pool[1024];
    static std::vector<Node*> allocated;
};

#endif
