#ifndef GAME_CLIENT_RENDER_LEVELRENDERER_HPP
#define GAME_CLIENT_RENDER_LEVELRENDERER_HPP

#include <string>
#include <utility>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/client/render/ParticleType.hpp"
#include "game/network/LevelListener.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/util/AABB.hpp"
#include "game/core/Culler.hpp"
#include "game/core/HitResult.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/Chunk.hpp"
#include "game/world/block/BlockDestructionProgress.hpp"

class BlockEntity;
class BlockState;
class Chunk;
class ClipChunk;
class IconRegister;
class Minecraft;
class MultiPlayerLevel;
class Player;
class ItemInstance;
class ParticleType;
class Textures;
class Entity;
class Tesselator;
class EntityRenderDispatcher;
class BlockEntityRenderDispatcher;
class Font;

class LevelRenderer : public LevelListener
{
public:
    class DestroyedBlockManager
    {
    public:
        class RecentBlock
        {
        public:
            RecentBlock(const BlockPos& pos, Level* level);
            ~RecentBlock();

            BlockPos pos;
            Level* level;
            std::vector<AABB*> boxes;
            int ticks;
            bool remove;
        };

        DestroyedBlockManager();
        ~DestroyedBlockManager();
        void tick();
        void updatedChunkAt(Level* level, const BlockPos& pos, int radius);
        void destroyingBlockAt(Level* level, const BlockPos& pos);
        void addAABBs(Level* level, const AABB* area, std::vector<AABB*>* result);

    private:
        OSFastMutex lock;
        std::vector<RecentBlock*> blocks;
    };

    LevelRenderer(Minecraft* minecraft, Textures* textures);
    ~LevelRenderer();

    static unsigned int getGlobalChunkCount();
    void renderStars();
    void createCloudMesh();
    unsigned char activePlayers();
    void nonStackDirtyChunksAdded();
    void resortChunks(int x, int y, int z, int dimension);
    void allChanged(int dimension);
    void setLevel(int dimension, MultiPlayerLevel* level);
    void tick();
    void clear();
    void allChanged();
    static bool sortEntDisFunc(std::pair<int, double> a, std::pair<int, double> b);
    bool shouldEntityGlow(boost::shared_ptr<Entity> entity, boost::shared_ptr<Entity> camera, Culler* culler);
    static int getDimensionIndexFromId(int id);
    static bool isGlobalIndexInSameDimension(int index, Level* level);
    void setupDestroyState();
    void restoreDestroyState();
    void renderEntities(boost::shared_ptr<Entity> camera, Culler* culler, float partialTick);
    unsigned int getBucketMask(double x, double y, double z, ClipChunk* chunk);
    void setupCameraInsideAreas(boost::shared_ptr<Entity> camera);
    unsigned int clippedByCameraInsideAreas(ClipChunk* chunk);
    void renderChunks(int pass, int layer, int count, double distance);
    void render(boost::shared_ptr<Entity> camera, int pass, double distance, bool translucent);
    void renderSky(float partialTick);
    void renderHaloRing(float partialTick);
    void renderAdvancedClouds(float partialTick, int pass, double x, double y, double z);
    void renderClouds(float partialTick, int pass, double x, double y, double z);
    bool isInCloud(double x, double y, double z, float partialTick);
    void setGlobalChunkFlag(int index, unsigned char mask, unsigned char value);
    static unsigned long long getGlobalIndexForChunk(int x, int z, int dimension, Level* level);
    bool getGlobalChunkFlag(int x, int z, int dimension, Level* level, unsigned char mask, unsigned char value);
    unsigned int updateDirtyChunks(unsigned int* count);
    void renderHit(boost::shared_ptr<Player> player, const HitResult* hit, int pass, not_null_ptr<ItemInstance> item, float partialTick);
    void renderDestroyAnimation(Tesselator* tesselator, boost::shared_ptr<Player> player, float partialTick);
    static void renderLineBox(const AABB* box, int color);
    void renderHitOutline(boost::shared_ptr<Player> player, const HitResult* hit, int pass, float partialTick);
    static void renderColoredLineBox(const AABB* box, int color, bool depth);
    static void renderLine(std::vector<GameRenderer::PositionStruct>* positions, std::vector<int>* colors, std::vector<int>* indices, float width);
    static void renderColoredBox(const AABB* box, int color, int alpha);
    static void renderStringInWorld(boost::shared_ptr<Entity> entity, const std::wstring& text, Vec3* position);
    static void renderStringOnPlane(boost::shared_ptr<Entity> entity, const std::wstring& text, Vec3* origin, Vec3* normal);
    static void renderDebugVolume(boost::shared_ptr<Entity> entity, const std::wstring& text, const AABB* box, int color, bool a, bool b, bool c, bool d);
    static void renderFancyTextDebugVolume(boost::shared_ptr<Entity> entity, const std::wstring& text, const AABB* box, int color, bool a, bool b, bool c);
    static void renderDebugWireframeVolume(boost::shared_ptr<Entity> entity, const std::wstring& text, const AABB* box, int color, bool a, bool b, bool c, bool d);
    void setDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, Level* level);
    void blockChanged(Level* level, const BlockPos& pos, const BlockState* oldState, const BlockState* newState, int flags, bool immediate);
    void blockLightChanged(const BlockPos& pos);
    void setBlocksDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, Level* level);
    void cull(Culler* culler, float partialTick);
    void notifyNearbyEntities(Level* level, const BlockPos& pos, bool immediate);
    void playStreamingMusic(const SoundEvent* sound, const BlockPos& pos, bool playing);
    void playSound(boost::shared_ptr<Player> player, const SoundEvent* sound, SoundSource::ESoundSource source, double x, double y, double z, float volume, float pitch, float range);
    void addParticle(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params);
    int calculateParticleLevel(Level* level, bool overrideLimiter);
    void addParticleInternal(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params);
    void addParticleInternal(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, bool force, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params);
    void addParticle(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, bool force, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params);
    void addParticle(Level* level, const ParticleType* particle, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params);
    void entityAdded(boost::shared_ptr<Entity> entity);
    void entityRemoved(boost::shared_ptr<Entity> entity);
    void globalLevelEvent(int type, const BlockPos& pos, int data);
    void levelEvent(boost::shared_ptr<Player> player, int type, const BlockPos& pos, int data);
    void destroyBlockProgress(int breakerId, const BlockPos& pos, int progress);
    void registerTextures(IconRegister* iconRegister);
    void fullyFlagRenderableBlockEntitiesToBeRemoved();
    static unsigned long long getGlobalIndexForChunk(int x, int z, int dimension, int unused);
    void setGlobalChunkFlag(int x, int z, int dimension, Level* level, unsigned char mask, unsigned char value);
    void clearGlobalChunkFlag(int x, int z, int dimension, Level* level, unsigned char mask, unsigned char value);
    unsigned char getGlobalChunkFlags(int x, int z, int dimension, Level* level);
    unsigned long long setGlobalChunkFlags(int x, int z, int dimension, Level* level, unsigned char flags);
    unsigned char incGlobalChunkRefCount(int x, int z, int dimension, Level* level);
    unsigned char decGlobalChunkRefCount(int x, int z, int dimension, Level* level);
    int checkAllPresentChunks(bool* present);
    void playerRemoved(boost::shared_ptr<Entity> player);

    void AddDLCSkinsToMemTextures();
    void gatherStats1();
    void gatherStats2();
    void renderSameAsLast(int pass, double distance);
    static void renderColoredLineBox(const AABB* box, int a, int b, int c, int d, bool depth);
    static void renderLine(std::vector<Vec3*>* points, int color);
    static void renderLine(std::vector<Vec3*>* points, std::vector<int>* colors, std::vector<int>* indices, float width);
    void skyColorChanged();
    static unsigned int getGlobalChunkCountForOverworld();

private:
    ChunkBlockEntityMap mBlockEntities;
    OSFastMutex mBlockEntityMutex;
    Level* mLevels[4];
    Textures* mTextures;
    arrayWithLength<ClipChunk> mClipChunks[8];
    int mLastViewDistance;
    bool mSkyColorChanged;
    int mDisplayList;
    int mStarList;
    int mSkyList;
    Minecraft* mMinecraft;
    EntityRenderDispatcher* mEntityRenderDispatcher;
    BlockEntityRenderDispatcher* mBlockEntityRenderDispatcher;
    Font* mFont;
    int mChunkGridWidth;
    int mChunkGridHeight;
    int mChunkGridDepth;
    std::vector<Chunk*> mChunks;
    OSFastMutex mChunkMutex;
    boost::unordered_set<boost::shared_ptr<BlockEntity> > mRenderableBlockEntities;
    double mCameraX;
    double mCameraY;
    double mCameraZ;
    double mLastCameraX;
    double mLastCameraY;
    double mLastCameraZ;
    int mRenderTick;
    boost::unordered_map<int, BlockDestructionProgress*> mDestroyingBlocks;
    std::vector<AABB*> mTempAABBs;
    OSFastMutex mDirtyMutex;
    DestroyedBlockManager* mDestroyedBlocks;
    float mCloudOffset;
    unsigned char* mGlobalChunkFlags;
    OSFastMutex mGlobalChunkFlagsMutex;
};

#endif
