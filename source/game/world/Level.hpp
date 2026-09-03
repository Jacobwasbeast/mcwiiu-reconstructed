#ifndef GAME_WORLD_LEVEL_HPP
#define GAME_WORLD_LEVEL_HPP

#include <vector>
#include <string>
#include <typeinfo>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>

#include "game/util/Vec3.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelSource.hpp"
#include "game/core/BoundingBox.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/GameRules.hpp"

class LevelStorage;
class LevelData;
class Dimension;
class LevelChunk;
class ChunkSource;
class Biome;
class BiomeSource;
class LevelSettings;
class Block;
class BlockState;
class Direction;
class Entity;
class EntityTickCache;
class LevelListener;
class SoundEvent;
class ParticleType;
class Player;
class MinecraftServer;
class PathNavigationListener;
class Random;
class WorldBorder;
class SavedDataStorage;
class SavedData;
class HitResult;
class BlockEntity;
class CompoundTag;
class Packet;
class PlayerUID;
class LevelGenerationOptions;
class LevelType;
class LevelSource;
class BiomeSource;
struct CriticalSection;
enum ePARTICLE_TYPE;
namespace SoundSource { enum ESoundSource; }
template<typename T> struct arrayWithLength;

class Level : public LevelSource
{
public:
    class RangeModifier
    {
    public:
        ~RangeModifier();
    };

    Level(boost::shared_ptr<LevelStorage> levelStorage, LevelData* levelData, Dimension* dimension, bool isClientSide);
    ~Level();
    void _init();
    bool isWaterAt(const BlockPos& pos);
    bool shouldFreeze(const BlockPos& pos, bool checkNeighbours);
    bool shouldFreezeIgnoreNeighbors(const BlockPos& pos);
    long long getSeed();
    LevelData* getLevelData();
    void clampWithinLevelBounds(double& x, double& z, double margin);
    bool isWithinLevelBounds(int x, int z);
    void clampWithinLevelHeight(double& y, double margin);
    static void enableLightingCache();
    static void destroyLightingCache();
    static bool getInstaTick();
    static void setInstaTick(bool instaTick);
    bool hasEntitiesToRemove();
    void postConstruct();
    LevelChunk* getChunk(int chunkX, int chunkZ);
    LevelChunk* getChunkAt(const BlockPos& pos);
    bool hasChunkAt(const BlockPos& pos, bool allowEmpty);
    bool hasChunkAt(const BlockPos& pos);
    Biome* getBiome(const BlockPos& pos);
    BiomeSource* getBiomeSource();
    void initializeLevel(LevelSettings* settings);
    MinecraftServer* getServer();
    void setSpawnPos(const BlockPos& pos);
    void validateSpawn();
    int getSeaLevel();
    const BlockState* getTopBlockState(const BlockPos& pos);
    bool isInWorldBounds(const BlockPos& pos);
    bool isInWorldBounds(int x, int y, int z);
    bool isOutsideBuildHeight(const BlockPos& pos);
    bool isEmptyBlock(const BlockPos& pos);
    bool isEmptyWaterBlock(const BlockPos& pos);
    bool hasChunksAt(const BlockPos& pos, int radius, bool allowEmpty);
    bool hasChunksAt(const BlockPos& pos, int radius);
    bool hasChunksAt(const BlockPos& from, const BlockPos& to, bool allowEmpty);
    bool hasChunksAt(const BlockPos& from, const BlockPos& to);
    bool hasChunksAt(BoundingBox box, bool allowEmpty);
    bool hasChunksAt(BoundingBox box);
    bool reallyHasChunk(int chunkX, int chunkZ);
    bool reallyHasChunkAt(const BlockPos& pos);
    bool reallyHasChunksAt(const BlockPos& from, const BlockPos& to);
    bool hasChunksAt(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, bool allowEmpty);
    LevelChunk* getChunkAt(int x, int z);
    LevelChunk* getChunkLoadedOrUnloadedAt(int x, int z);
    bool isChunkGeneratedAt(int chunkX, int chunkZ);
    bool setBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate);
    bool setExtraBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate);
    void setBlockBrightnessChanged(const BlockPos& pos);
    void setBrightness(LightLayer::variety layer, const BlockPos& pos, int brightness, bool markDirty);
    int getBrightnessCached(unsigned int* cache, LightLayer::variety layer, int x, int y, int z);
    bool canSeeSky(const BlockPos& pos);
    int getEmissionCached(unsigned int* cache, Block* block, const BlockPos& pos);
    int getExpectedLight(unsigned int* cache, const BlockPos& pos, LightLayer::variety layer, bool ignoreEmission);
    int getBlockingCached(unsigned int* cache, LightLayer::variety layer, Block* block, Block* extraBlock, const BlockPos& pos);
    void initCachePartial(unsigned int* cache, int x, int y, int z);
    void initCacheComplete(unsigned int* cache, int x, int y, int z);
    void setBrightnessCached(unsigned int* cache, unsigned long long* dirty, LightLayer::variety layer, const BlockPos& pos, int brightness);
    void setBlocksDirty(const BlockPos& from, const BlockPos& to);
    void flushCache(unsigned int* cache, unsigned long long dirty, LightLayer::variety layer);
    bool checkLight(LightLayer::variety layer, const BlockPos& pos, bool onlyLoadedChunks, bool skipCentre);
    bool checkLight(const BlockPos& pos, bool onlyLoadedChunks, bool skipCentre);
    void neighborChanged(const BlockPos& pos, Block* block, const BlockPos& fromPos);
    void observedNeighborChanged(const BlockPos& pos, Block* block, const BlockPos& fromPos);
    void updateObservingBlocksAt(const BlockPos& pos, Block* block);
    void updateNeighborsAt(const BlockPos& pos, Block* block, bool updateObservers);
    void blockUpdated(const BlockPos& pos, Block* block, bool updateObservers);
    bool internalSetBlock(bool extraBlock, const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate);
    bool setData(const BlockPos& pos, int data, int flags, bool forceUpdate);
    bool setData(int x, int y, int z, int data, int flags, bool forceUpdate);
    bool setBlockAndData(int x, int y, int z, int blockId, int data, int flags);
    bool removeBlock(const BlockPos& pos);
    void levelEvent(boost::shared_ptr<Player> player, int type, const BlockPos& pos, int data);
    void levelEvent(int type, const BlockPos& pos, int data);
    bool destroyBlock(const BlockPos& pos, bool dropResources);
    bool setBlockAndUpdate(const BlockPos& pos, const BlockState* state);
    void sendBlockUpdated(const BlockPos& pos, const BlockState* oldState, const BlockState* newState, int flags);
    void lightColumnChanged(int x, int z, int fromY, int toY);
    void updateNeighborsAtExceptFromFacing(const BlockPos& pos, Block* block, const Direction* skipSide);
    bool isBlockToBeTickedAt(const BlockPos& pos, Block* block);
    bool canSeeSkyFromBelowWater(const BlockPos& pos);
    int getDaytimeRawBrightness(const BlockPos& pos);
    int getRawBrightness(const BlockPos& pos, bool useNeighbourBrightness);
    int getRawBrightness(const BlockPos& pos);
    int getHeightmapHeight(int x, int z);
    BlockPos getHeightmapPos(const BlockPos& pos);
    int getHeightmap(int x, int z);
    int getLowestHeightmap(int x, int z);
    int getBrightnessPropagate(LightLayer::variety layer, const BlockPos& pos);
    void getNeighbourBrightnesses(int* brightnesses, LightLayer::variety layer, int x, int y, int z);
    int getBrightness(LightLayer::variety layer, const BlockPos& pos);
    int getLightColor(const BlockPos& pos, int lightValue, int cacheIndex);
    float getBrightness(const BlockPos& pos, int lightValue);
    float getBrightness(const BlockPos& pos);
    const BlockState* getBlockState(const BlockPos& pos);
    const BlockState* getBlockState(EntityTickCache& tickCache, const BlockPos& pos);
    const BlockState* getBlockState(LevelChunk* chunk, EntityTickCache& tickCache, const BlockPos& pos);
    int getBlockId(int x, int y, int z);
    int getBlockId(EntityTickCache& tickCache, int x, int y, int z);
    unsigned short getElement(EntityTickCache& tickCache, int x, int y, int z);
    unsigned short getElement(int x, int y, int z);
    unsigned short getExtraElement(int x, int y, int z);
    void getElementAndExtraElement(int x, int y, int z, unsigned short* element, unsigned short* extraElement);
    bool sectionContainsBlockId(int x, int y, int z, int blockId);
    int getExtraBlockId(int x, int y, int z);
    void getBlockAndExtraBlockId(int x, int y, int z, int* blockId, int* extraBlockId);
    Block* getExtraBlock(const BlockPos& pos);
    void getBlockAndExtraBlock(const BlockPos& pos, Block** block, Block** extraBlock);
    Block* getLiquidBlock(const BlockPos& pos);
    const BlockState* getLiquidBlockState(const BlockPos& pos);
    const BlockState* getExtraBlockState(const BlockPos& pos);
    int getBlockData(int x, int y, int z);
    int getExtraBlockData(int x, int y, int z);
    Block* getBlock(const BlockPos& pos);
    bool isSolidRender(const BlockPos& pos);
    bool isDay();
    HitResult* clip(Vec3* from, Vec3* to, bool stopOnLiquid, bool ignoreBlocksWithoutShape, bool returnLastUncollidableBlock, bool testUseBlocks, boost::unordered_set<Block*, boost::hash<Block*>, std::equal_to<Block*>, std::allocator<Block*> >* alwaysClipBlocks, bool clipExtraBlocks);
    HitResult* clip(Vec3* from, Vec3* to);
    HitResult* clip(Vec3* from, Vec3* to, bool stopOnLiquid);
    void playSoundToAllPlayers(const BlockPos& pos, const SoundEvent* sound, SoundSource::ESoundSource source, float volume, float pitch, float range);
    void playSound(boost::shared_ptr<Player> player, const BlockPos& pos, const SoundEvent* sound, SoundSource::ESoundSource source, float volume, float pitch, float range);
    void playSound(boost::shared_ptr<Player> player, double x, double y, double z, const SoundEvent* sound, SoundSource::ESoundSource source, float volume, float pitch, float range);
    void playLocalSound(double x, double y, double z, const SoundEvent* sound, SoundSource::ESoundSource source, float volume, float pitch, bool distanceDelay, float range);
    void playStreamingMusic(const BlockPos& pos, const SoundEvent* sound, bool playing);
    void addParticle(ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params);
    void addParticle(const ParticleType* type, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params);
    void addAlwaysVisibleParticle(ePARTICLE_TYPE type, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params);
    void addParticle(const ParticleType* type, bool overrideLimiter, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params);
    bool addGlobalEntity(boost::shared_ptr<Entity> entity);
    bool addEntity(boost::shared_ptr<Entity> entity);
    void entityAdded(boost::shared_ptr<Entity> entity);
    void entityRemoved(boost::shared_ptr<Entity> entity);
    void playerRemoved(boost::shared_ptr<Entity> player);
    void removeEntity(boost::shared_ptr<Entity> entity);
    void removeEntityImmediately(boost::shared_ptr<Entity> entity);
    void addListener(LevelListener* listener);
    void removeListener(LevelListener* listener);
    bool isInsideWorldBorder(boost::shared_ptr<Entity> entity);
    float getTimeOfDay(float partialTick);
    float getRainLevel(float partialTick);
    float getThunderLevel(float partialTick);
    float getOldSkyDarken(float partialTick);
    float getSkyDarken(float partialTick);
    int getMoonPhase();
    float getMoonBrightness();
    float getSunAngle(float partialTick);
    int getCloudColor(float partialTick);
    bool biomeHasRain(const BlockPos& pos);
    bool biomeHasSnow(const BlockPos& pos);
    float getStarBrightness(float partialTick);
    const Difficulty* getDifficulty();
    DifficultyInstance getCurrentDifficultyAt(const BlockPos& pos);
    int getSkyDarken();
    void setSkyDarken(int value);
    int getSkyFlashTime();
    void setSkyFlashTime(int value);
    bool isFindingSpawn();
    void setFindingSpawn(bool value);
    GameRules* getGameRules();
    int getMaxBuildHeight();
    int getHeight();
    WorldBorder* getWorldBorder();
    int getCloudHeight();
    int getFogDistance();
    void setFogDistance(int value);
    bool useNewSeaLevel();
    bool isFlatLand();
    bool AllPlayersAreSleeping();
    bool isSpawnChunk(int chunkX, int chunkZ);
    bool getHasBeenInCreative();
    bool isGenerateMapFeatures();
    BlockPos getTopRainBlockPos(const BlockPos& pos);
    bool isSnowTemperature(const BlockPos& pos);
    bool isRainingAt(const BlockPos& pos);
    bool isSnowingAt(const BlockPos& pos);
    void setThunderLevel(float value);
    void setRainLevel(float value);
    bool isThundering();
    bool isRaining();
    void setSavedDataStorage(SavedDataStorage* storage);
    SavedDataStorage* getSavedDataStorage();
    void setSavedData(const std::wstring& id, boost::shared_ptr<SavedData> data);
    boost::shared_ptr<SavedData> getSavedData(const std::type_info& type, const std::wstring& id);
    int getFreeAuxValueFor(const std::wstring& id);
    int getAuxValueForMap(PlayerUID uid, int dimension, int x, int z, int scale);
    void globalLevelEvent(int type, const BlockPos& pos, int data);
    Random getRandomFor(int x, int y, int z);
    static Random fjGetRandomFor(long long seed, int x, int y, int z);
    bool isAllEmpty();
    int getHorizonHeight();
    void destroyBlockProgress(int breakerId, const BlockPos& pos, int progress);
    void setSeaLevel(int value);
    LevelType* getGeneratorType();
    void setSpawnSettings(bool spawnPeaceful, bool spawnHostile);
    int getDirectSignal(const BlockPos& pos, const Direction* direction);
    int getDirectSignalTo(const BlockPos& pos);
    int getSignal(const BlockPos& pos, const Direction* direction);
    bool hasSignal(const BlockPos& pos, const Direction* direction);
    bool hasNeighborSignal(const BlockPos& pos);
    int getBestNeighborSignal(const BlockPos& pos);
    void setGameTime(long long value);
    long long getGameTime();
    long long getDayTime();
    void setDayTime(long long value);
    void forceDayTime(long long value);
    bool ShouldForceTimeUpdate();
    BlockPos getSharedSpawnPos();
    ChunkSource* getChunkSource();
    boost::shared_ptr<LevelStorage> getLevelStorage();
    unsigned char getWeatherState();
private:
    boost::shared_ptr<LevelStorage> mLevelStorage;
    LevelData* mLevelData;
    Dimension* mDimension;
    LevelChunk* mChunkSource;
    Random* mRandom;
    PathNavigationListener* mPathListener;
    WorldBorder* mWorldBorder;
    SavedDataStorage* mSavedDataStorage;
    unsigned int* mLightUpdateQueue;
    CriticalSection mEntityMutex;
    CriticalSection mBlockEntityMutex;
    CriticalSection mLightMutex;
    std::vector<boost::shared_ptr<Entity> > mEntities;
    std::vector<boost::shared_ptr<Entity> > mEntitiesToRemove;
    std::vector<boost::shared_ptr<Entity> > mGlobalEntities;
    std::vector<boost::shared_ptr<Player> > mPlayers;
    std::vector<LevelListener*> mListeners;
    int mSeaLevel;
    int mCloudColour;
    int mSkyDarken;
    int mRandValue;
    int mAddend;
    int mSkyFlashTime;
    float mOldRainLevel;
    float mRainLevel;
    float mThunderLevel;
    float mOldThunderLevel;
    bool mIsClientSide;
    bool mScheduledUpdatesAreImmediate;
    bool mSpawnPeacefulMobs;
    bool mSpawnHostileMobs;
    bool mFindingSpawn;
    bool mProcessingBlockEntities;
    bool mRemovingBlockEntity;
    bool mForceTimeUpdate;
    bool mHasDirtyRegion;
    int mAmbientTickCountdown;
    int mFogDistance;
    int mDirtyMinX;
    int mDirtyMaxX;
    int mDirtyMinY;
    int mDirtyMaxY;
    int mDirtyMinZ;
    int mDirtyMaxZ;
    void* mLootTables;
};

#endif
