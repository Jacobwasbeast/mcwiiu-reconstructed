#include "game/server/ServerLevel.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/PlayerList.hpp"

ServerLevel::ServerLevel(MinecraftServer* server, boost::shared_ptr<LevelStorage> storage, LevelData* data, int dimension)
    : Level(storage, data, 0, false), server(server), tracker(0), chunkMap(0), chunkSource(0), emptyTime(0), blockEventCount(0), allPlayersSleeping(false), updatingBlocks(false), portalForcer(0), blockEntityCount(0), villageSiegeTick(0), enderCrystalCount(0), saving(false), tickCount(0), weatherTick(0)
{
}

ServerLevel::~ServerLevel()
{
}

uint8_t ServerLevel::AllPlayersAreSleeping()
{
    return allPlayersSleeping;
}

EntityTracker* ServerLevel::getTracker()
{
    return tracker;
}

void ServerLevel::getChunkSource()
{
}

std::vector<boost::shared_ptr<BlockEntity> >* ServerLevel::getBlockEntitiesInRegion(int32_t first, int32_t second, int32_t third, int32_t fourth, int32_t fifth, int32_t sixth)
{
    return 0;
}

PlayerChunkMap* ServerLevel::getChunkMap()
{
    return chunkMap;
}

void ServerLevel::getDimensionSpecificSpawn()
{
}

PortalForcer* ServerLevel::getPortalForcer()
{
    return portalForcer;
}

uint32_t ServerLevel::runUpdate(uint32_t first)
{
    return 0;
}

uint64_t ServerLevel::staticCtor()
{
    return 0;
}

uint64_t ServerLevel::createChunkSource()
{
    return 0;
}

ServerLevel* ServerLevel::postConstruct()
{
    return 0;
}

uint32_t ServerLevel::allPlayersAreSleeping()
{
    return 0;
}

uint64_t ServerLevel::stopWeather()
{
    return 0;
}

uint64_t ServerLevel::awakenAllPlayers()
{
    return 0;
}

void ServerLevel::save(bool first, ProgressListener* second, bool third)
{
}

uint32_t ServerLevel::doBlockEvent(BlockEventData* first)
{
    return 0;
}

uint64_t ServerLevel::runBlockEvents()
{
    return 0;
}

void ServerLevel::runQueuedSendBlockUpdates()
{
}

void ServerLevel::tick()
{
}

uint32_t ServerLevel::getRandomMobSpawnAt(MobCategory* first, BlockPos* second)
{
    return 0;
}

bool ServerLevel::canMobSpawnAt(MobCategory* first, Biome::MobSpawnerData* second, BlockPos* third)
{
    return false;
}

void ServerLevel::updateSleepingPlayerList()
{
}

void ServerLevel::validateSpawn()
{
}

uint64_t ServerLevel::hasChunk(int32_t first, int32_t second, bool third)
{
    return 0;
}

uint64_t ServerLevel::getChunkIfHasChunk(int32_t first, int32_t second, bool third)
{
    return 0;
}

void ServerLevel::findLightingTargetAround(BlockPos* first)
{
}

uint64_t ServerLevel::tickBlocks()
{
    return 0;
}

bool ServerLevel::isBlockToBeTickedAt(BlockPos* first, Block* second)
{
    return false;
}

bool ServerLevel::hasTickNextTick(BlockPos* first, Block* second)
{
    return false;
}

void ServerLevel::addToTickNextTick(BlockPos* first, Block* second, int32_t third)
{
}

void ServerLevel::addToTickNextTick(BlockPos* first, Block* second, int32_t third, int32_t fourth)
{
}

void ServerLevel::forceAddBlockTick(BlockPos* first, Block* second, int32_t third, int32_t fourth)
{
}

void ServerLevel::resetEmptyTime()
{
    emptyTime = 0;
}

void ServerLevel::tickEntities()
{
}

uint64_t ServerLevel::tickServerPlayers()
{
    return 0;
}

bool ServerLevel::tickPendingTicks(bool first)
{
    return false;
}

uint64_t ServerLevel::fetchTicksInChunk(LevelChunk* first, bool second)
{
    return 0;
}

std::vector<TickNextTickData>* ServerLevel::fetchTicksInArea(BoundingBox* first, bool second)
{
    return 0;
}

void ServerLevel::isAnimalsAllowed()
{
}

void ServerLevel::isNpcsAllowed()
{
}

uint64_t ServerLevel::tick(boost::shared_ptr<Entity> first, bool second)
{
    return 0;
}

uint64_t ServerLevel::forceTick(boost::shared_ptr<Entity> first, bool second)
{
    return 0;
}

uint32_t ServerLevel::mayInteract(boost::shared_ptr<Player> first, BlockPos* second, Block* third)
{
    return 0;
}

void ServerLevel::initializeLevel(LevelSettings* first)
{
}

void ServerLevel::setupDebugLevel()
{
}

uint64_t ServerLevel::generateBonusItemsNearSpawn()
{
    return 0;
}

uint64_t ServerLevel::setInitialSpawn(LevelSettings* first)
{
    return 0;
}

uint64_t ServerLevel::Suspend()
{
    return 0;
}

void ServerLevel::saveToDisc(ProgressListener* first, bool second)
{
}

uint32_t ServerLevel::removeDuplicateAndPendingEntity(boost::shared_ptr<Entity> first)
{
    return 0;
}

void ServerLevel::addEntities(std::vector<boost::shared_ptr<Entity> >* first)
{
}

uint64_t ServerLevel::saveLevelData()
{
    return 0;
}

uint64_t ServerLevel::entityAddedExtra(boost::shared_ptr<Entity> first)
{
    return 0;
}

void ServerLevel::entityAdded(boost::shared_ptr<Entity> first)
{
}

uint64_t ServerLevel::entityRemovedExtra(boost::shared_ptr<Entity> first)
{
    return 0;
}

void ServerLevel::entityRemoved(boost::shared_ptr<Entity> first)
{
}

uint64_t ServerLevel::getEntity(uint32_t first, uint32_t second)
{
    return 0;
}

uint32_t ServerLevel::addGlobalEntity(boost::shared_ptr<Entity> first)
{
    return 0;
}

void ServerLevel::broadcastEntityEvent(boost::shared_ptr<Entity> first, uint8_t second, int32_t third)
{
}

uint64_t ServerLevel::explode(uint64_t first, uint64_t second, uint64_t third, uint64_t fourth, uint32_t fifth, uint32_t sixth, uint32_t seventh, int32_t eighth)
{
    return 0;
}

uint64_t ServerLevel::blockEvent(BlockPos* first, Block* second, int32_t third, int32_t fourth)
{
    return 0;
}

uint64_t ServerLevel::closeLevelStorage()
{
    return 0;
}

uint64_t ServerLevel::tickWeather(bool first)
{
    return 0;
}

void ServerLevel::getChunkPollRange()
{
}

MinecraftServer* ServerLevel::getServer()
{
    return server;
}

uint64_t ServerLevel::setTimeAndAdjustBlockTicks(uint32_t first, int32_t second, uint32_t third)
{
    return 0;
}

void ServerLevel::sendParticles(boost::shared_ptr<ServerPlayer> first, bool second, uint32_t third)
{
}

uint64_t ServerLevel::sendParticles(double first, double second, double third, double fourth, double fifth, double sixth, double seventh, uint32_t eighth, uint32_t ninth, uint32_t tenth, uint32_t eleventh)
{
    return 0;
}

uint64_t ServerLevel::sendParticles(uint32_t first, uint32_t second, uint32_t third)
{
    return 0;
}

void ServerLevel::sendParticles(const ParticleType* type, double x, double y, double z, int count, double xDist, double yDist, double zDist, double maxSpeed, arrayWithLength<int> params, double xAux, double yAux, double zAux)
{
}

uint64_t ServerLevel::sendParticles(double first, double second, double third, double fourth, double fifth, double sixth, double seventh, uint32_t eighth, uint32_t ninth, uint32_t tenth, uint32_t eleventh, uint32_t twelfth)
{
    return 0;
}

void ServerLevel::getEntityByUUID(uint32_t first, uint32_t second)
{
}

uint64_t ServerLevel::findNearestMapFeature(std::wstring* first, BlockPos* second, bool third)
{
    return 0;
}

boost::shared_ptr<ServerPlayer> ServerLevel::getPlayer(const PlayerUID& playerUID) const
{
    return server->getPlayers()->getPlayer(playerUID);
}

void ServerLevel::queueSendBlockUpdate(int32_t first, int32_t second, int32_t third, BlockState* fourth, BlockState* fifth)
{
}

bool ServerLevel::atBlockEntityLimit(int32_t first, uint32_t second)
{
    return false;
}

uint32_t ServerLevel::setBlock(BlockPos* first, BlockState* second, int32_t third, bool fourth)
{
    return 0;
}

uint32_t ServerLevel::addEntity(boost::shared_ptr<Entity> first)
{
    return 0;
}

bool ServerLevel::atEntityLimit(boost::shared_ptr<Entity> first)
{
    return false;
}

bool ServerLevel::newPrimedTntAllowed()
{
    return false;
}

bool ServerLevel::newFallingBlockAllowed()
{
    return false;
}

bool ServerLevel::explodeNextEnderCrystal()
{
    return false;
}

void ServerLevel::flagEntitiesToBeRemoved(uint32_t* first, bool* second)
{
}

Scoreboard* ServerLevel::GetScoreboard()
{
    return 0;
}

Scoreboard* ServerLevel::GetServerScoreboard()
{
    return 0;
}

uint64_t ServerLevel::invalidateChunksOutsideArea(int32_t first, int32_t second, int32_t third, int32_t fourth)
{
    return 0;
}

void ServerLevel::blockEntityAddedExtra(boost::shared_ptr<BlockEntity> first)
{
}

void ServerLevel::blockEntityRemovedExtra(boost::shared_ptr<BlockEntity> first)
{
}




