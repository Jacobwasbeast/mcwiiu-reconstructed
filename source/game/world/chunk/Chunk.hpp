#ifndef GAME_WORLD_CHUNK_CHUNK_HPP
#define GAME_WORLD_CHUNK_CHUNK_HPP

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/CriticalSection.hpp"

class AABB;
class BlockEntity;
class Culler;
class ClipChunk;
class Entity;
class Level;

struct IntKeyHash
{
    std::size_t operator()(int value) const;
};

struct IntKeyEq
{
    bool operator()(int first, int second) const;
};

typedef boost::unordered_map<int, std::vector<boost::shared_ptr<BlockEntity> >, IntKeyHash, IntKeyEq> ChunkBlockEntityMap;

class Chunk
{
public:
    Chunk(Level* level, ChunkBlockEntityMap& blockEntities, OSFastMutex& mutex, int x, int z, int dimension, ClipChunk* clipChunk);
    Chunk();
    ~Chunk();

    void reset();
    void setPos(int x, int z, int dimension);
    void translateToPos();
    unsigned int elementToBlockId(unsigned short element);
    void makeCopyForRebuild(Chunk* source);
    unsigned long long rebuild(unsigned int* count);
    double distanceToSqr(boost::shared_ptr<Entity> entity) const;
    void _delete();
    unsigned long long cull(Culler* culler);
    void clearDirty();

    unsigned int elementToData(unsigned short element);
    double squishedDistanceToSqr(boost::shared_ptr<Entity> entity);
    std::vector<boost::shared_ptr<BlockEntity> >& getList(int key);
    void renderBB();
    bool isEmpty();
    void setDirty();
    bool emptyFlagSet(int flag);

    Level* mLevel;
    int mX;
    int mZ;
    int mDimension;
    int mDirty;
    int mEmpty;
    int mRenderable;
    int mMinX;
    int mMinZ;
    int mMinDimension;
    int mMaxX;
    int mMaxZ;
    int mMaxDimension;
    AABB* mBoundingBox;
    bool mOwnsBoundingBox;
    ClipChunk* mClipChunk;
    int mUnknown;
    ChunkBlockEntityMap* mBlockEntities;
    OSFastMutex* mMutex;
    bool mNonStack;
};

#endif
