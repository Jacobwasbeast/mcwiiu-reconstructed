#ifndef GAME_CORE_NODE_HPP
#define GAME_CORE_NODE_HPP

#include <string>

class BlockPathTypes;
class BlockPos;
class DataInputStream;
class DataOutputStream;

class Node
{
public:
    int x;
    int y;
    int z;
    int hash;
    int heapIdx;
    float g;
    float h;
    float f;
    Node* cameFrom;
    bool closed;
    float walkedDistance;
    float moveCost;
    float costMalus;
    const BlockPathTypes* type;
    bool reached;
    int searchId;

    Node();
    Node(int x, int y, int z);
    Node(const Node& other);

    static void ResetPool();
    static Node* GetNodeFromPool(int x, int y, int z);
    static int createHash(int x, int y, int z);
    static Node* createFromStream(DataInputStream* stream);

    void _init();

    int GetType();

    Node* cloneMove(int x, int y, int z);

    float distanceTo(Node* node);
    float distanceToSqr(Node* node);
    float distanceManhattan(Node* node);
    float distanceManhattan(const BlockPos& pos);

    bool equals(Node* node);
    int hashCode();
    bool inOpenSet();

    std::wstring toString();

    void writeToStream(DataOutputStream* stream);
};

#endif
