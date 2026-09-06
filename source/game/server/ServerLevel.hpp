#include <cstdint>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include "game/client/render/ParticleType.hpp"
#include "game/world/Level.hpp"
#include "game/core/TickNextTickData.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/biome/Biome.hpp"
class MinecraftServer; class PlayerChunkMap; class PortalForcer; class ProgressListener; class BlockEventData; class Block; class BlockState; class LevelChunk; class BoundingBox; class MobCategory; class LevelSettings; class Entity; class EntityTracker; class Player; class ServerPlayer; class ClientboundLevelParticlesPacket; class BlockEntity; class PlayerUID; class LevelStorage; class Scoreboard;

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



class ServerLevel : public Level {
public:
 struct QueuedSendBlockData { int x; int y; int z; const BlockState* oldState; const BlockState* newState; };
 
 MinecraftServer* server;
 EntityTracker* tracker;
 PlayerChunkMap* chunkMap;
 void* chunkSource;
 int32_t emptyTime;
 std::vector<TickNextTickData> pendingTicks;
 boost::unordered_map<std::wstring, boost::shared_ptr<Entity> > entitiesByUuid;
 std::vector<QueuedSendBlockData*> queuedBlockUpdates;
 int32_t blockEventCount;
 bool allPlayersSleeping;
 bool updatingBlocks;
 PortalForcer* portalForcer;
 int32_t blockEntityCount;
 int32_t villageSiegeTick;
 int32_t enderCrystalCount;
 bool saving;
 int32_t tickCount;
 int32_t weatherTick;
 std::vector<TickNextTickData> scheduledTicks;

 
 ServerLevel(MinecraftServer* first, boost::shared_ptr<LevelStorage> second, LevelData* third, int32_t fourth); 
 ~ServerLevel(); 
 uint8_t AllPlayersAreSleeping(); 
 EntityTracker* getTracker(); 
 void getChunkSource();
 std::vector<boost::shared_ptr<BlockEntity> >* getBlockEntitiesInRegion(int32_t first, int32_t second, int32_t third, int32_t fourth, int32_t fifth, int32_t sixth); 
 PlayerChunkMap* getChunkMap(); 
 void getDimensionSpecificSpawn(); 
 PortalForcer* getPortalForcer(); 
 static uint32_t runUpdate(uint32_t first); 
 static uint64_t staticCtor(); 
 uint64_t createChunkSource(); 
 ServerLevel* postConstruct(); 
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
 uint64_t hasChunk(int32_t first, int32_t second, bool third); 
 uint64_t getChunkIfHasChunk(int32_t first, int32_t second, bool third); 
 void findLightingTargetAround(BlockPos* first); 
 uint64_t tickBlocks(); 
 bool isBlockToBeTickedAt(BlockPos* first, Block* second); 
 bool hasTickNextTick(BlockPos* first, Block* second); 
 void addToTickNextTick(BlockPos* first, Block* second, int32_t third); 
 void addToTickNextTick(BlockPos* first, Block* second, int32_t third, int32_t fourth); 
 void forceAddBlockTick(BlockPos* first, Block* second, int32_t third, int32_t fourth); 
 void resetEmptyTime(); 
 void tickEntities(); 
 uint64_t tickServerPlayers(); 
 bool tickPendingTicks(bool first); 
 uint64_t fetchTicksInChunk(LevelChunk* first, bool second); 
 std::vector<TickNextTickData>* fetchTicksInArea(BoundingBox* first, bool second); 
 void isAnimalsAllowed();
 void isNpcsAllowed();
 uint64_t tick(boost::shared_ptr<Entity> first, bool second); 
 uint64_t forceTick(boost::shared_ptr<Entity> first, bool second); 
 uint32_t mayInteract(boost::shared_ptr<Player> first, BlockPos* second, Block* third); 
 void initializeLevel(LevelSettings* first); 
 void setupDebugLevel(); 
 uint64_t generateBonusItemsNearSpawn(); 
 uint64_t setInitialSpawn(LevelSettings* first); 
 uint64_t Suspend(); 
 void saveToDisc(ProgressListener* first, bool second); 
 uint32_t removeDuplicateAndPendingEntity(boost::shared_ptr<Entity> first); 
 void addEntities(std::vector<boost::shared_ptr<Entity> >* first); 
 uint64_t saveLevelData(); 
 uint64_t entityAddedExtra(boost::shared_ptr<Entity> first); 
 void entityAdded(boost::shared_ptr<Entity> first); 
 uint64_t entityRemovedExtra(boost::shared_ptr<Entity> first); 
 void entityRemoved(boost::shared_ptr<Entity> first); 
 uint64_t getEntity(uint32_t first, uint32_t second); 
 uint32_t addGlobalEntity(boost::shared_ptr<Entity> first); 
 void broadcastEntityEvent(boost::shared_ptr<Entity> first, uint8_t second, int32_t third); 
 uint64_t explode(uint64_t first, uint64_t second, uint64_t third, uint64_t fourth, uint32_t fifth, uint32_t sixth, uint32_t seventh, int32_t eighth); 
 uint64_t blockEvent(BlockPos* first, Block* second, int32_t third, int32_t fourth); 
 uint64_t closeLevelStorage(); 
 uint64_t tickWeather(bool first); 
 void getChunkPollRange(); 
 MinecraftServer* getServer(); 
 uint64_t setTimeAndAdjustBlockTicks(uint32_t first, int32_t second, uint32_t third); 
 void sendParticles(boost::shared_ptr<ServerPlayer> first, bool second, uint32_t third); 
 uint64_t sendParticles(double first, double second, double third, double fourth, double fifth, double sixth, double seventh, uint32_t eighth, uint32_t ninth, uint32_t tenth, uint32_t eleventh); 
 uint64_t sendParticles(uint32_t first, uint32_t second, uint32_t third); 
 void sendParticles(const ParticleType* type, double x, double y, double z, int count, double xDist, double yDist, double zDist, double maxSpeed, arrayWithLength<int> params, double xAux, double yAux, double zAux); 
 uint64_t sendParticles(double first, double second, double third, double fourth, double fifth, double sixth, double seventh, uint32_t eighth, uint32_t ninth, uint32_t tenth, uint32_t eleventh, uint32_t twelfth); 
 void getEntityByUUID(uint32_t first, uint32_t second); 
 uint64_t findNearestMapFeature(std::wstring* first, BlockPos* second, bool third); 
 boost::shared_ptr<ServerPlayer> getPlayer(const PlayerUID& playerUID) const; 
 void queueSendBlockUpdate(int32_t first, int32_t second, int32_t third, BlockState* fourth, BlockState* fifth); 
 bool atBlockEntityLimit(int32_t first, uint32_t second); 
 uint32_t setBlock(BlockPos* first, BlockState* second, int32_t third, bool fourth); 
 uint32_t addEntity(boost::shared_ptr<Entity> first); 
 bool atEntityLimit(boost::shared_ptr<Entity> first); 
 bool newPrimedTntAllowed(); 
 bool newFallingBlockAllowed(); 
 bool explodeNextEnderCrystal(); 
 void flagEntitiesToBeRemoved(uint32_t* first, bool* second); 
 Scoreboard* GetScoreboard();
 Scoreboard* GetServerScoreboard();
 uint64_t invalidateChunksOutsideArea(int32_t first, int32_t second, int32_t third, int32_t fourth); 
 void blockEntityAddedExtra(boost::shared_ptr<BlockEntity> first); 
 void blockEntityRemovedExtra(boost::shared_ptr<BlockEntity> first); 
 
    bool isEntityIdInUse(int value);
};

#endif




