#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include "game/client/render/ParticleType.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/world/Level.hpp"
#include "game/core/TickNextTickData.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/biome/Biome.hpp"
class MinecraftServer; class PlayerChunkMap; class PortalForcer; class ProgressListener; class BlockEventData; class Block; class BlockState; class LevelChunk; class BoundingBox; class MobCategory; class LevelSettings; class Entity; class EntityTracker; class Player; class ServerPlayer; class ServerScoreboard; class ClientboundLevelParticlesPacket; class BlockEntity; class PlayerUID; class LevelStorage; class Scoreboard; class Explosion;

#ifndef GAME_SERVER_SERVERLEVEL_HPP
#define GAME_SERVER_SERVERLEVEL_HPP










struct Block;
struct BlockEventData;
struct BlockPos;
struct BlockState;
struct BoundingBox;
struct LevelChunk;
struct LevelData;
struct LevelSettings;
struct MinecraftServer;
struct MobCategory;
struct PlayerChunkMap;
struct PortalForcer;
struct ProgressListener;

struct TickNextTickDataKeyCompare
{
    bool operator()(const TickNextTickData& first, const TickNextTickData& second) const
    {
        return TickNextTickData::compare_fnct(first, second);
    }
};

struct TickNextTickDataKeyHash
{
    std::size_t operator()(const TickNextTickData& data) const
    {
        return TickNextTickData::hash_fnct(data);
    }
};

struct TickNextTickDataKeyEq
{
    bool operator()(const TickNextTickData& first, const TickNextTickData& second) const
    {
        return TickNextTickData::eq_test(first, second);
    }
};

class ServerLevel : public Level {
public:
 struct QueuedSendBlockData { int x; int y; int z; const BlockState* oldState; const BlockState* newState; };
 
 MinecraftServer* server;
 EntityTracker* tracker;
 PlayerChunkMap* chunkMap;
 void* chunkSource;
 int32_t emptyTime;
 OSFastMutex tickNextTickMutex;
 std::set<TickNextTickData, TickNextTickDataKeyCompare> tickNextTickList;
 boost::unordered_set<TickNextTickData, TickNextTickDataKeyHash, TickNextTickDataKeyEq> tickNextTickSet;
 boost::unordered_map<std::wstring, boost::shared_ptr<Entity> > entitiesByUuid;
 std::vector<QueuedSendBlockData*> queuedBlockUpdates;
 std::vector<BlockEventData*> blockEvents;
 int32_t blockEventCount;
 bool allPlayersSleeping;
 bool updatingBlocks;
 PortalForcer* portalForcer;
 int32_t blockEntityCount;
 int32_t villageSiegeTick;
 int32_t enderCrystalCount;
 bool saving;
 ServerScoreboard* serverScoreboard;
 int32_t weatherTick;
 std::vector<TickNextTickData> toBeTicked;

 
 ServerLevel(MinecraftServer* first, boost::shared_ptr<LevelStorage> second, LevelData* third, int32_t fourth); 
 ~ServerLevel(); 
 bool AllPlayersAreSleeping(); 
 EntityTracker* getTracker(); 
 void getChunkSource();
 std::vector<boost::shared_ptr<BlockEntity> >* getBlockEntitiesInRegion(int32_t first, int32_t second, int32_t third, int32_t fourth, int32_t fifth, int32_t sixth); 
 PlayerChunkMap* getChunkMap(); 
 BlockPos* getDimensionSpecificSpawn();
 PortalForcer* getPortalForcer(); 
 static int runUpdate(void* first);
 static void staticCtor();
 ChunkSource* createChunkSource(); 
 void postConstruct(); 
 uint32_t allPlayersAreSleeping(); 
 uint64_t stopWeather(); 
 uint64_t awakenAllPlayers(); 
 void save(bool first, ProgressListener* second, bool third); 
 uint32_t doBlockEvent(BlockEventData* first); 
 uint64_t runBlockEvents(); 
 void runQueuedSendBlockUpdates(); 
 void tick(); 
 uint32_t getRandomMobSpawnAt(MobCategory* first, BlockPos* second); 
 bool canMobSpawnAt(MobCategory* first, Biome::MobSpawnerData* second, BlockPos* third); 
 void updateSleepingPlayerList(); 
 void validateSpawn(); 
 bool hasChunk(int32_t first, int32_t second, bool third); 
 LevelChunk* getChunkIfHasChunk(int32_t first, int32_t second, bool third); 
 void findLightingTargetAround(BlockPos* first); 
 void tickBlocks();
 bool isBlockToBeTickedAt(const BlockPos& pos, Block* block);
 bool hasTickNextTick(const BlockPos& pos, Block* block);
 void addToTickNextTick(const BlockPos& pos, Block* block, int delay);
 void addToTickNextTick(const BlockPos& pos, Block* block, int delay, int priority);
 void forceAddBlockTick(const BlockPos& pos, Block* block, int delay, int priority);
 void resetEmptyTime(); 
 void tickEntities(); 
 void tickServerPlayers(); 
 bool tickPendingTicks(bool runAll);
 std::vector<TickNextTickData>* fetchTicksInChunk(LevelChunk* chunk, bool removeTicks);
 std::vector<TickNextTickData>* fetchTicksInArea(BoundingBox* box, bool removeTicks);
 void isAnimalsAllowed();
 void isNpcsAllowed();
 void tick(boost::shared_ptr<Entity> first, bool second); 
 uint64_t forceTick(boost::shared_ptr<Entity> first, bool second); 
 uint32_t mayInteract(boost::shared_ptr<Player> first, BlockPos* second, Block* third); 
 void initializeLevel(LevelSettings* first); 
 void setupDebugLevel(); 
 uint64_t generateBonusItemsNearSpawn(); 
 virtual void setInitialSpawn(LevelSettings* first); 
 uint64_t Suspend(); 
 void saveToDisc(ProgressListener* first, bool second); 
 uint32_t removeDuplicateAndPendingEntity(boost::shared_ptr<Entity> first); 
 void addEntities(std::vector<boost::shared_ptr<Entity> >* first); 
 virtual void saveLevelData(); 
 uint64_t entityAddedExtra(boost::shared_ptr<Entity> first); 
 void entityAdded(boost::shared_ptr<Entity> first); 
 uint64_t entityRemovedExtra(boost::shared_ptr<Entity> first); 
 void entityRemoved(boost::shared_ptr<Entity> first); 
 uint64_t getEntity(uint32_t first, uint32_t second); 
 bool addGlobalEntity(boost::shared_ptr<Entity> first); 
 void broadcastEntityEvent(boost::shared_ptr<Entity> first, uint8_t second, int32_t third); 
 boost::shared_ptr<Explosion> explode(boost::shared_ptr<Entity> entity, double x, double y, double z, float radius, bool fire, bool smoke, bool blockInteraction, bool particles, bool sounds, float damageMultiplier);
 uint64_t blockEvent(BlockPos* first, Block* second, int32_t third, int32_t fourth); 
 uint64_t closeLevelStorage(); 
 void tickWeather(bool first); 
 int getChunkPollRange(); 
 MinecraftServer* getServer(); 
 uint64_t setTimeAndAdjustBlockTicks(uint32_t first, int32_t second, uint32_t third); 
 void sendParticles(boost::shared_ptr<ServerPlayer> player, bool longDistance, boost::shared_ptr<ClientboundLevelParticlesPacket> packet);
 void sendParticles(const ParticleType* type, bool longDistance, double x, double y, double z, int count, double xDist, double yDist, double zDist, double maxSpeed, arrayWithLength<int> params);
 void sendParticles(const ParticleType* type, double x, double y, double z, int count, double xDist, double yDist, double zDist, double maxSpeed, arrayWithLength<int> params);
 void sendParticles(const ParticleType* type, double x, double y, double z, int count, double xDist, double yDist, double zDist, double maxSpeed, arrayWithLength<int> params, double xAux, double yAux, double zAux);
 void sendParticles(boost::shared_ptr<ServerPlayer> player, const ParticleType* type, bool longDistance, double x, double y, double z, int count, double xDist, double yDist, double zDist, double maxSpeed, arrayWithLength<int> params); 
 void getEntityByUUID(uint32_t first, uint32_t second); 
 uint64_t findNearestMapFeature(std::wstring* first, BlockPos* second, bool third); 
 boost::shared_ptr<ServerPlayer> getPlayer(const PlayerUID& playerUID) const; 
 void queueSendBlockUpdate(int32_t first, int32_t second, int32_t third, BlockState* fourth, BlockState* fifth); 
 bool atBlockEntityLimit(int32_t first, uint32_t second); 
 uint32_t setBlock(BlockPos* first, BlockState* second, int32_t third, bool fourth); 
 bool addEntity(boost::shared_ptr<Entity> first); 
 bool atEntityLimit(boost::shared_ptr<Entity> first); 
 bool newPrimedTntAllowed(); 
 bool newFallingBlockAllowed(); 
 bool explodeNextEnderCrystal(); 
 void flagEntitiesToBeRemoved(uint32_t* first, bool* second); 
 Scoreboard* GetScoreboard();
 ServerScoreboard* GetServerScoreboard();
 uint64_t invalidateChunksOutsideArea(int32_t first, int32_t second, int32_t third, int32_t fourth); 
 void blockEntityAddedExtra(boost::shared_ptr<BlockEntity> first); 
 void blockEntityRemovedExtra(boost::shared_ptr<BlockEntity> first); 
 
    bool isEntityIdInUse(int value);
};

#endif
