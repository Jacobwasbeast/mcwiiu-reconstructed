#include "game/server/ServerLevel.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/ServerScoreboard.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/ClientboundExplodePacket.hpp"
#include "game/core/Explosion.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/entity/mob/SkeletonHorse.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/SkeletonHorse.hpp"
#include "game/core/LightningBolt.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/world/block/BlockEventData.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/core/BoundingBox.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/core/CriticalSection.hpp"
#include "platform/wiiu/C4JEventArrayImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/server/DerivedServerLevel.hpp"
#include "game/data/SavedDataStorage.hpp"
#include "game/network/ClientboundLevelParticlesPacket.hpp"

extern void instructionSynchronize();

namespace
{
C4JEventArrayImpl* sBlockUpdateEvents = 0;
OSFastMutex sBlockUpdateMutex[3];
C4JThreadImpl* sBlockUpdateThread = 0;
ServerLevel* sBlockUpdateLevels[3] = { 0, 0, 0 };
int sBlockUpdateChunkCounts[3] = { 0, 0, 0 };
int sBlockUpdateChunkX[3][0xB48];
int sBlockUpdateChunkZ[3][0xB48];
unsigned int sBlockUpdateTickCounts[3] = { 0, 0, 0 };
int sBlockUpdateRandom[3] = { 0, 0, 0 };
int sBlockUpdateTickX[3][256];
int sBlockUpdateTickY[3][256];
int sBlockUpdateTickZ[3][256];

int blockUpdateDimension(const ServerLevel* level)
{
    int id = level->mDimension->getType()->getId();
    return id == -1 ? 1 : (id == 1 ? 2 : 0);
}

bool queueBlockTick(ServerLevel* level, int x, int y, int z,
                     int id, int data, int dimension, unsigned int& grassCount,
                     unsigned int& lavaCount, unsigned int& accepted)
{
    if (id == 0 || accepted >= 256)
        return false;
    if ((id == 2 && grassCount >= 100) || (id == 11 && lavaCount >= 100))
        return false;
    Block* block = Block::byId(id);
    if (block == 0 || !block->isTicking())
        return false;
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (Minecraft::InMiniGame(static_cast<EMiniGameId>(15), true)
        && (miniGame == 0 || !miniGame->shouldTickBlock(id)))
        return false;
    const BlockState* state = block->getBlockState(data);
    BlockPos position(x, y, z);
    if (!block->shouldBlockTick(level, position, state))
        return false;
    sBlockUpdateTickX[dimension][accepted] = x;
    sBlockUpdateTickY[dimension][accepted] = y;
    sBlockUpdateTickZ[dimension][accepted] = z;
    ++accepted;
    if (block == Blocks::grass && grassCount < 100)
        ++grassCount;
    if (block == Blocks::lava && lavaCount < 100)
        ++lavaCount;
    return true;
}
}

ServerLevel::ServerLevel(MinecraftServer* server, boost::shared_ptr<LevelStorage> storage, LevelData* data, int dimension)
    : Level(storage, data, 0, false), server(server), tracker(0), chunkMap(0), chunkSource(0), emptyTime(0), tickNextTickSet(11), blockEventCount(0), allPlayersSleeping(false), updatingBlocks(false), portalForcer(0), blockEntityCount(0), villageSiegeTick(0), enderCrystalCount(0), saving(false), serverScoreboard(0), weatherTick(0)
{
    InitializeCriticalSection(&tickNextTickMutex);
}

ServerLevel::~ServerLevel()
{
}

bool ServerLevel::AllPlayersAreSleeping()
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
    (void)first; (void)second; (void)third; (void)fourth; (void)fifth; (void)sixth;
    return 0;
}

PlayerChunkMap* ServerLevel::getChunkMap()
{
    return chunkMap;
}

BlockPos* ServerLevel::getDimensionSpecificSpawn()
{
    return mDimension->getSpawnPos();
}

PortalForcer* ServerLevel::getPortalForcer()
{
    return portalForcer;
}

int ServerLevel::runUpdate(void* first)
{
    (void)first;
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_ServerLevel);
    ShutdownManager::HasStarted(ShutdownManager::eThreadId_ServerLevelUpdate, sBlockUpdateEvents);
    while (ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ServerLevelUpdate))
    {
        for (int dimension = 0; dimension < 3; ++dimension)
        {
            sBlockUpdateEvents->WaitForSingle(dimension, -1);
            PIXBeginNamedEvent(0.0f, "Updating blocks to be ticked %d", dimension);
            if (!ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ServerLevelUpdate))
                break;
            const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
            if (Minecraft::InMiniGame(static_cast<EMiniGameId>(15), true)
                && (miniGame == 0 || !miniGame->shouldTickBlocks()))
            {
                PIXEndNamedEvent();
                continue;
            }
            EnterCriticalSection(&sBlockUpdateMutex[dimension]);
            ServerLevel* level = sBlockUpdateLevels[dimension];
            unsigned int accepted = 0;
            unsigned int grassCount = 0;
            unsigned int lavaCount = 0;
            int count = sBlockUpdateChunkCounts[dimension];
            for (int index = 0; level != 0 && index < count; ++index)
            {
                int chunkX = sBlockUpdateChunkX[dimension][index];
                int chunkZ = sBlockUpdateChunkZ[dimension][index];
                if (!level->hasChunk(chunkX, chunkZ, true))
                    continue;
                int minX = 0, maxX = 15, minZ = 0, maxZ = 15;
                if (!level->hasChunk(chunkX + 1, chunkZ, true)) maxX = 11;
                if (!level->hasChunk(chunkX, chunkZ + 1, true)) maxZ = 11;
                if (!level->hasChunk(chunkX - 1, chunkZ, true)) minX = 4;
                if (!level->hasChunk(chunkX, chunkZ - 1, true)) minZ = 4;
                if (!level->hasChunk(chunkX + 1, chunkZ + 1, true)) { maxX = 11; maxZ = 11; }
                if (!level->hasChunk(chunkX + 1, chunkZ - 1, true)) { maxX = 11; minZ = 4; }
                if (!level->hasChunk(chunkX - 1, chunkZ - 1, true)) { minX = 4; minZ = 4; }
                if (!level->hasChunk(chunkX - 1, chunkZ + 1, true)) { minX = 4; maxZ = 11; }
                LevelChunk* chunk = level->getChunk(chunkX, chunkZ);
                if (chunk == 0) continue;
                chunk->getElementReadLock();
                for (int sample = 0; sample < 0x50 && accepted < 256; ++sample)
                {
                    int random = sBlockUpdateRandom[dimension] * 3 + level->mAddend;
                    sBlockUpdateRandom[dimension] = random;
                    unsigned int packed = static_cast<unsigned int>(random >> 2);
                    int localX = packed & 15;
                    int localZ = (packed >> 8) & 15;
                    if (localX < minZ || localX > minX || localZ < maxX || localZ > maxZ)
                        continue;
                    int localY = (packed >> 16) & 255;
                    int id = 0, extraId = 0;
                    chunk->getBlockAndExtraId_NoLock(localX, localY, localZ, &id, &extraId);
                    bool queued = queueBlockTick(level, chunkX * 16 + localX, localY,
                                                 chunkZ * 16 + localZ, id,
                                                 chunk->getData(localX, localY, localZ), dimension,
                                                 grassCount, lavaCount, accepted);
                    if (!queued)
                        queueBlockTick(level, chunkX * 16 + localX, localY,
                                       chunkZ * 16 + localZ, extraId,
                                       chunk->getExtraBlockData(localX, localY, localZ), dimension,
                                       grassCount, lavaCount, accepted);
                }
                chunk->releaseElementReadLock();
            }
            sBlockUpdateTickCounts[dimension] = accepted;
            sBlockUpdateChunkCounts[dimension] = 0;
            LeaveCriticalSection(&sBlockUpdateMutex[dimension]);
            PIXEndNamedEvent();
            if (level != 0)
            {
                PIXBeginNamedEvent(0.0f, "Level::tickBlocks");
                level->Level::tickBlocks();
                PIXEndNamedEvent();
            }
        }
    }
    ShutdownManager::HasFinished(ShutdownManager::eThreadId_ServerLevelUpdate);
    instructionSynchronize();
    return 0;
}

void ServerLevel::staticCtor()
{
    sBlockUpdateEvents = new C4JEventArrayImpl(3, C4JEventArray::Mode0);
    InitializeCriticalSection(&sBlockUpdateMutex[0]);
    InitializeCriticalSection(&sBlockUpdateMutex[1]);
    InitializeCriticalSection(&sBlockUpdateMutex[2]);
    sBlockUpdateThread = new C4JThreadImpl(&ServerLevel::runUpdate, 0, "Block update", 0);
    sBlockUpdateThread->SetPriority(2);
    sBlockUpdateThread->Run();
}

ChunkSource* ServerLevel::createChunkSource()
{
    ChunkStorage* storage = getLevelStorage()->createChunkStorage(mDimension);
    ChunkGenerator* generator = mDimension->createRandomLevelGenerator();
    return new ServerChunkCache(this, storage, generator);
}

void ServerLevel::postConstruct()
{
    Level::postConstruct();
}

uint32_t ServerLevel::allPlayersAreSleeping()
{
    return allPlayersSleeping ? 1U : 0U;
}

uint64_t ServerLevel::stopWeather()
{
    weatherTick = 0;
    return 1;
}

uint64_t ServerLevel::awakenAllPlayers()
{
    allPlayersSleeping = false;
    return 1;
}

void ServerLevel::save(bool first, ProgressListener* second, bool third)
{
    saving = true;
    if (chunkSource)
        static_cast<ServerChunkCache*>(chunkSource)->save(first, second);
    saving = false;
    (void)third;
}

uint32_t ServerLevel::doBlockEvent(BlockEventData* first)
{
    if (!first)
        return 0;
    Block* block = first->getBlock();
    if (!block)
        return 0;
    const BlockPos position = first->getPos();
    const BlockState* state = getBlockState(position);
    return state && block->triggerEvent(state, this, position, first->getParamA(), first->getParamB()) ? 1U : 0U;
}

uint64_t ServerLevel::runBlockEvents()
{
    blockEventCount = 0;
    return 1;
}

void ServerLevel::runQueuedSendBlockUpdates()
{
    for (std::vector<QueuedSendBlockData*>::iterator it = queuedBlockUpdates.begin(); it != queuedBlockUpdates.end(); ++it)
        delete *it;
    queuedBlockUpdates.clear();
}

void ServerLevel::tick()
{
    tickWeather(true);
    tickBlocks();
    tickEntities();
}

uint32_t ServerLevel::getRandomMobSpawnAt(MobCategory* first, BlockPos* second)
{
    return canMobSpawnAt(first, 0, second) ? 1U : 0U;
}

bool ServerLevel::canMobSpawnAt(MobCategory* first, Biome::MobSpawnerData* second, BlockPos* third)
{
    return first != 0 && second != 0 && third != 0;
}

void ServerLevel::updateSleepingPlayerList()
{
    allPlayersSleeping = false;
}

void ServerLevel::validateSpawn()
{
    Level::validateSpawn();
}

bool ServerLevel::hasChunk(int32_t first, int32_t second, bool third)
{
    return chunkSource != 0 && static_cast<ServerChunkCache*>(chunkSource)->hasChunk(first, second);
}

LevelChunk* ServerLevel::getChunkIfHasChunk(int32_t first, int32_t second, bool third)
{
    return Level::getChunkSource()->getChunkIfHasChunk(first, second);
}

void ServerLevel::findLightingTargetAround(BlockPos* first)
{
    if (first)
        checkLight(*first, true, false);
}

void ServerLevel::tickBlocks()
{
    int dimension = blockUpdateDimension(this);
    unsigned int accepted = sBlockUpdateTickCounts[dimension];
    for (unsigned int index = 0; index < accepted; ++index)
    {
        BlockPos position(sBlockUpdateTickX[dimension][index],
                          sBlockUpdateTickY[dimension][index],
                          sBlockUpdateTickZ[dimension][index]);
        if (!hasChunkAt(position))
            continue;
        LevelChunk* chunk = getChunk(position.x >> 4, position.z >> 4);
        if (chunk == 0)
            continue;
        const BlockState* state = chunk->getBlockState(position.x & 15, position.y, position.z & 15);
        if (state != 0 && state->getBlock() != 0 && state->getBlock()->isTicking())
            state->getBlock()->randomTick(this, position, state, mRandom);
        const BlockState* extraState = chunk->getExtraBlockState(position);
        if (extraState != 0 && extraState->getBlock() != 0 && extraState->getBlock()->isTicking())
            extraState->getBlock()->randomTick(this, position, extraState, mRandom);
    }
    sBlockUpdateTickCounts[dimension] = 0;
    Level::tickBlocks();
    EnterCriticalSection(&sBlockUpdateMutex[dimension]);
    sBlockUpdateLevels[dimension] = this;
    sBlockUpdateRandom[dimension] = mRandValue;
    int count = mChunksToPoll.end();
    for (int index = 0; index < count && sBlockUpdateChunkCounts[dimension] < 0xB48; ++index)
    {
        ChunkPos chunkPos = mChunksToPoll.get(index);
        if (!hasChunk(chunkPos.x, chunkPos.z, true))
            continue;
        int queueIndex = sBlockUpdateChunkCounts[dimension]++;
        sBlockUpdateChunkX[dimension][queueIndex] = chunkPos.x;
        sBlockUpdateChunkZ[dimension][queueIndex] = chunkPos.z;
        LevelChunk* chunk = getChunk(chunkPos.x, chunkPos.z);
        if (chunk != 0)
        {
            chunk->tick(false);
            if (mRandom->nextInt(16) == 0)
            {
                BlockPos rainPosition = getTopRainBlockPos(BlockPos(chunkPos.x * 16 + mRandom->nextInt(16), 0,
                                                                     chunkPos.z * 16 + mRandom->nextInt(16)));
                BlockPos below = rainPosition.below();
                if (shouldFreeze(below, true))
                    setBlockAndUpdate(below, Blocks::ice->defaultBlockState());
            }
            if (mRandom->nextInt(16) == 0 && isRaining() && isThundering())
            {
                int lightningState = mRandValue * 3 + mAddend;
                mRandValue = lightningState;
                unsigned int packed = static_cast<unsigned int>(lightningState >> 2);
                BlockPos lightningPosition(chunkPos.x * 16 + (packed & 15), 0,
                                           chunkPos.z * 16 + ((packed >> 8) & 15));
                findLightingTargetAround(&lightningPosition);
                if (isRainingAt(lightningPosition))
                {
                    if (!getGameRules()->getBoolean(3))
                    {
                        boost::shared_ptr<LightningBolt> bolt(new LightningBolt(
                            this, lightningPosition.x, lightningPosition.y,
                            lightningPosition.z, false));
                        addGlobalEntity(boost::static_pointer_cast<Entity>(bolt));
                    }
                    else
                    {
                        DifficultyInstance difficulty = getCurrentDifficultyAt(lightningPosition);
                        if (mRandom->nextDouble() < static_cast<double>(difficulty.getEffectiveDifficulty()) * 0.01)
                        {
                            boost::shared_ptr<SkeletonHorse> horse(new SkeletonHorse(this));
                            horse->setTrap(true);
                            horse->setPos(lightningPosition.x, lightningPosition.y, lightningPosition.z);
                            addGlobalEntity(boost::static_pointer_cast<Entity>(horse));
                            boost::shared_ptr<LightningBolt> bolt(new LightningBolt(
                                this, lightningPosition.x, lightningPosition.y,
                                lightningPosition.z, true));
                            addGlobalEntity(boost::static_pointer_cast<Entity>(bolt));
                        }
                    }
                }
            }
            if (mRandom->nextInt(8) == 0)
            {
                BlockPos lightPosition(chunkPos.x * 16 + mRandom->nextInt(16),
                                       mRandom->nextInt(0x80),
                                       chunkPos.z * 16 + mRandom->nextInt(16));
                checkLight(lightPosition, false, false);
            }
        }
    }
    mChunksToPoll.clear();
    LeaveCriticalSection(&sBlockUpdateMutex[dimension]);
    sBlockUpdateEvents->Set(dimension);
    instructionSynchronize();
}

bool ServerLevel::isBlockToBeTickedAt(const BlockPos& pos, Block* block)
{
    TickNextTickData data(pos, block);
    std::vector<TickNextTickData>::iterator it = toBeTicked.begin();
    while (it != toBeTicked.end())
    {
        if (*it == data)
        {
            break;
        }
        ++it;
    }
    return it != toBeTicked.end();
}

bool ServerLevel::hasTickNextTick(const BlockPos& pos, Block* block)
{
    TickNextTickData data(pos, block);
    return tickNextTickSet.find(data) != tickNextTickSet.end();
}

void ServerLevel::addToTickNextTick(const BlockPos& pos, Block* block, int delay)
{
    addToTickNextTick(pos, block, delay, 0);
}

void ServerLevel::addToTickNextTick(const BlockPos& pos, Block* block, int delay, int priority)
{
    MemSect(0x1b);
    if (Level::getInstaTick() && block->defaultBlockState()->getMaterial() != Material::AIR)
    {
        if (block->canInstantlyTick())
        {
            if (hasChunksAt(pos.offset(-8, -8, -8), pos.offset(8, 8, 8)))
            {
                const BlockState* state = getBlockState(pos);
                if (state->getMaterial() != Material::AIR && state->getBlock() == block)
                {
                    state->getBlock()->tick(this, pos, state, mRandom);
                    MemSect(0);
                    return;
                }
            }
            MemSect(0);
            return;
        }
        delay = 1;
    }

    TickNextTickData data(pos, block);
    if (hasChunkAt(pos))
    {
        if (block->getCraftingMaterial() != Material::AIR)
        {
            data.delay(getGameTime() + delay);
            data.setPriorityTilt(priority);
        }
        EnterCriticalSection(&tickNextTickMutex);
        if (tickNextTickSet.find(data) == tickNextTickSet.end())
        {
            tickNextTickSet.insert(data);
            tickNextTickList.insert(data);
        }
        LeaveCriticalSection(&tickNextTickMutex);
    }
    MemSect(0);
}

void ServerLevel::forceAddBlockTick(const BlockPos& pos, Block* block, int delay, int priority)
{
    TickNextTickData data(pos, block);
    data.setPriorityTilt(priority);
    if (block->defaultBlockState()->getMaterial() != Material::AIR)
    {
        data.delay(getGameTime() + delay);
    }

    EnterCriticalSection(&tickNextTickMutex);
    if (tickNextTickSet.find(data) == tickNextTickSet.end())
    {
        tickNextTickSet.insert(data);
        tickNextTickList.insert(data);
    }
    LeaveCriticalSection(&tickNextTickMutex);
}

void ServerLevel::resetEmptyTime()
{
    emptyTime = 0;
}

void ServerLevel::tickEntities()
{
    resetEmptyTime();
    Level::tick();
}

void ServerLevel::tickServerPlayers()
{
    if (server && server->getPlayers())
        server->getPlayers()->tick();
}

bool ServerLevel::tickPendingTicks(bool runAll)
{
    EnterCriticalSection(&tickNextTickMutex);

    int count = (int)tickNextTickList.size();
    if (count > 1000)
    {
        count = 1000;
    }

    for (int i = 0; i < count; ++i)
    {
        std::set<TickNextTickData, TickNextTickDataKeyCompare>::iterator entry = tickNextTickList.begin();
        TickNextTickData data = *entry;
        if (!runAll && data.triggerTick > getGameTime())
        {
            break;
        }
        tickNextTickList.erase(entry);
        tickNextTickSet.erase(data);
        toBeTicked.push_back(data);
    }

    PIXBeginNamedEvent(1.035f, "Tick toBeTicked ticks (length=%i)", (int)toBeTicked.size());

    std::vector<TickNextTickData>::iterator it = toBeTicked.begin();
    while (it != toBeTicked.end())
    {
        TickNextTickData data = *it;
        it = toBeTicked.erase(it);
        if (!hasChunksAt(data.pos.offset(0, 0, 0), data.pos.offset(0, 0, 0)))
        {
            addToTickNextTick(data.pos, data.getBlock(), 0);
        }
        else
        {
            const BlockState* state = getBlockState(data.pos);
            Block* block = state->getBlock();
            const BlockState* extraState = getExtraBlockState(data.pos);
            Block* extraBlock = extraState->getBlock();
            if (block->getCraftingMaterial() != Material::AIR
                && Block::isMatching(block, data.getBlock()))
            {
                PIXBeginNamedEvent(1.035f, "Tick block '%i'", Block::getId(block));
                block->tick(this, data.pos, state, mRandom);
                PIXEndNamedEvent();
            }
            else if (extraBlock->getCraftingMaterial() != Material::AIR
                     && Block::isMatching(extraBlock, data.getBlock()))
            {
                PIXBeginNamedEvent(1.035f, "Tick Extra block '%i'", Block::getId(block));
                extraBlock->tick(this, data.pos, extraState, mRandom);
                PIXEndNamedEvent();
            }
        }
    }

    PIXEndNamedEvent();

    bool ticksRemaining = !tickNextTickList.empty();
    toBeTicked.clear();
    LeaveCriticalSection(&tickNextTickMutex);
    return ticksRemaining;
}

std::vector<TickNextTickData>* ServerLevel::fetchTicksInChunk(LevelChunk* chunk, bool removeTicks)
{
    ChunkPos chunkPos = chunk->getPos();
    BoundingBox box(chunkPos.x * 16 - 2, 0, chunkPos.z * 16 - 2,
                    chunkPos.x * 16 + 16, 256, chunkPos.z * 16 + 16);
    return fetchTicksInArea(&box, removeTicks);
}

std::vector<TickNextTickData>* ServerLevel::fetchTicksInArea(BoundingBox* box, bool removeTicks)
{
    EnterCriticalSection(&tickNextTickMutex);

    std::vector<TickNextTickData>* ticks = new std::vector<TickNextTickData>();

    for (int pass = 0; pass < 2; ++pass)
    {
        if (pass == 0)
        {
            std::set<TickNextTickData, TickNextTickDataKeyCompare>::iterator it = tickNextTickList.begin();
            while (it != tickNextTickList.end())
            {
                TickNextTickData data = *it;
                if (data.pos.x >= box->x0 && data.pos.x < box->x1
                    && data.pos.z >= box->z0 && data.pos.z < box->z1)
                {
                    if (removeTicks)
                    {
                        tickNextTickSet.erase(data);
                    }
                    else
                    {
                        ++it;
                    }
                    ticks->push_back(data);
                }
                else
                {
                    ++it;
                }
            }
        }
        else
        {
            std::vector<TickNextTickData>::iterator it = toBeTicked.begin();
            while (it != toBeTicked.end())
            {
                TickNextTickData data = *it;
                if (data.pos.x >= box->x0 && data.pos.x < box->x1
                    && data.pos.z >= box->z0 && data.pos.z < box->z1)
                {
                    if (removeTicks)
                    {
                        tickNextTickList.erase(data);
                        it = toBeTicked.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                    ticks->push_back(data);
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    LeaveCriticalSection(&tickNextTickMutex);
    return ticks;
}

void ServerLevel::isAnimalsAllowed()
{
}

void ServerLevel::isNpcsAllowed()
{
}

void ServerLevel::tick(boost::shared_ptr<Entity> first, bool second)
{
    (void)second;
    if (first)
        first->tick();
}

uint64_t ServerLevel::forceTick(boost::shared_ptr<Entity> first, bool second)
{
    Level::tick(first, second);
    return first ? 1U : 0U;
}

uint32_t ServerLevel::mayInteract(boost::shared_ptr<Player> first, BlockPos* second, Block* third)
{
    return first && second && third && isInWorldBounds(*second) ? 1U : 0U;
}

void ServerLevel::initializeLevel(LevelSettings* first)
{
    Level::initializeLevel(first);
}

void ServerLevel::setupDebugLevel()
{
    validateSpawn();
}

uint64_t ServerLevel::generateBonusItemsNearSpawn()
{
    return 1;
}

void ServerLevel::setInitialSpawn(LevelSettings* first)
{
    initializeLevel(first);
}

uint64_t ServerLevel::Suspend()
{
    saving = true;
    return 1;
}

void ServerLevel::saveToDisc(ProgressListener* first, bool second)
{
    save(second, first, true);
}

uint32_t ServerLevel::removeDuplicateAndPendingEntity(boost::shared_ptr<Entity> first)
{
    return first && !hasEntitiesToRemove() ? 1U : 0U;
}

void ServerLevel::addEntities(std::vector<boost::shared_ptr<Entity> >* first)
{
    if (!first)
        return;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = first->begin(); it != first->end(); ++it)
        addEntity(*it);
}

void ServerLevel::saveLevelData()
{
    checkSession();
    if (server != NULL)
    {
        for (unsigned int i = 0; i < server->mLevels.length; ++i)
        {
            DerivedServerLevel* derived = dynamic_cast<DerivedServerLevel*>(server->mLevels.data[i]);
            if (derived != NULL)
            {
                derived->saveDimensionData();
            }
        }
    }
    if (mLevelStorage)
    {
        mLevelStorage->saveLevelData(mLevelData, &mPlayers);
    }
    if (mSavedDataStorage)
    {
        mSavedDataStorage->save();
    }
}

uint64_t ServerLevel::entityAddedExtra(boost::shared_ptr<Entity> first)
{
    if (first)
        ++emptyTime;
    return first ? 1U : 0U;
}

void ServerLevel::entityAdded(boost::shared_ptr<Entity> first)
{
    if (first)
        Level::entityAdded(first);
}

uint64_t ServerLevel::entityRemovedExtra(boost::shared_ptr<Entity> first)
{
    if (first && emptyTime > 0)
        --emptyTime;
    return first ? 1U : 0U;
}

void ServerLevel::entityRemoved(boost::shared_ptr<Entity> first)
{
    if (first)
        Level::entityRemoved(first);
}

uint64_t ServerLevel::getEntity(uint32_t first, uint32_t second)
{
    return 0;
}

bool ServerLevel::addGlobalEntity(boost::shared_ptr<Entity> first)
{
    return first && Level::addGlobalEntity(first);
}

void ServerLevel::broadcastEntityEvent(boost::shared_ptr<Entity> first, uint8_t second, int32_t third)
{
    (void)first; (void)second; (void)third;
}

boost::shared_ptr<Explosion> ServerLevel::explode(boost::shared_ptr<Entity> entity, double x, double y, double z, float radius, bool fire, bool smoke, bool blockInteraction, bool particles, bool sounds, float damageMultiplier)
{
    boost::shared_ptr<Explosion> explosion(new Explosion(this, entity, x, y, z, radius, fire,
                                                          blockInteraction, false, !smoke,
                                                          !sounds, damageMultiplier, false));
    explosion->explode();
    explosion->finalizeExplosion(false, NULL);
    if (!fire)
        explosion->clearToBlow();
    if (!server || !server->getPlayers() || !explosion)
        return explosion;
    std::vector<boost::shared_ptr<ServerPlayer> >* players = server->getPlayers()->getPlayers();
    if (!players)
        return explosion;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin(); it != players->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        if (!player || !player->mConnection || player->mLevel != this)
            continue;
        const double dx = player->mX - x;
        const double dy = player->mY - y;
        const double dz = player->mZ - z;
        if (dx * dx + dy * dy + dz * dz >= 4096.0)
            continue;
        boost::shared_ptr<Player> asPlayer = player;
        Vec3* knockback = explosion->getHitPlayerKnockback(asPlayer);
        INetworkPlayer* candidate = player->mConnection->getNetworkPlayer();
        bool sameSystem = false;
        boost::shared_ptr<ServerPlayer> sourcePlayer = boost::dynamic_pointer_cast<ServerPlayer>(entity);
        if (candidate && sourcePlayer && sourcePlayer->mConnection)
        {
            INetworkPlayer* receiver = sourcePlayer->mConnection->getNetworkPlayer();
            sameSystem = receiver != 0 && receiver->IsSameSystem(candidate);
        }
        player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundExplodePacket(x, y, z, radius, explosion->getToBlow(), knockback, sameSystem)));
    }
    return explosion;
}

uint64_t ServerLevel::blockEvent(BlockPos* first, Block* second, int32_t third, int32_t fourth)
{
    if (!first || !second)
        return 0;
    ++blockEventCount;
    blockEvents.push_back(new BlockEventData(*first, second, third, fourth));
    return 1;
}

uint64_t ServerLevel::closeLevelStorage()
{
    boost::shared_ptr<LevelStorage> storage = getLevelStorage();
    if (storage)
        storage->closeAll();
    return 0;
}

void ServerLevel::tickWeather(bool first)
{
    const bool wasRaining = isRaining();
    const bool wasThundering = isThundering();
    Level::tickWeather(first);
    PlayerList* players = server == 0 ? 0 : server->getPlayers();
    if (players == 0)
        return;
    if (wasThundering != isThundering())
        players->broadcastAll(boost::shared_ptr<Packet>(new ClientboundGameEventPacket(8, 0.0f)), mDimension->getType()->getId());
    if (wasRaining != isRaining())
        players->broadcastAll(boost::shared_ptr<Packet>(new ClientboundGameEventPacket(2, 0.0f)), mDimension->getType()->getId());
}

int ServerLevel::getChunkPollRange()
{
    PlayerList* players = gMinecraftServer != NULL ? gMinecraftServer->getPlayers() : NULL;
    return players->getViewDistance();
}

MinecraftServer* ServerLevel::getServer()
{
    return server;
}

uint64_t ServerLevel::setTimeAndAdjustBlockTicks(uint32_t first, int32_t second, uint32_t third)
{
    (void)second;
    (void)third;
    setGameTime(static_cast<long long>(first));
    return 1;
}

void ServerLevel::sendParticles(boost::shared_ptr<ServerPlayer> player, bool longDistance, boost::shared_ptr<ClientboundLevelParticlesPacket> packet)
{
    if (!player || !packet)
        return;
    BlockPos pos = player->getCommandSenderBlockPosition();
    double distSqr = pos.distSqr(packet->getX(), packet->getY(), packet->getZ());
    if (distSqr <= 1024.0 || (longDistance && distSqr <= 262144.0))
    {
        player->mConnection->send(packet);
    }
}

void ServerLevel::sendParticles(const ParticleType* type, bool longDistance, double x, double y, double z, int count, double xDist, double yDist, double zDist, double maxSpeed, arrayWithLength<int> params)
{
    boost::shared_ptr<ClientboundLevelParticlesPacket> packet(
        new ClientboundLevelParticlesPacket(type, longDistance, (float)x, (float)y, (float)z, (float)xDist, (float)yDist, (float)zDist, (float)maxSpeed, count, params));
    for (size_t i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = boost::dynamic_pointer_cast<ServerPlayer>(mPlayers[i]);
        sendParticles(player, longDistance, packet);
    }
}

void ServerLevel::sendParticles(const ParticleType* type, double x, double y, double z, int count, double xDist, double yDist, double zDist, double maxSpeed, arrayWithLength<int> params)
{
    sendParticles(type, false, x, y, z, count, xDist, yDist, zDist, maxSpeed, params);
}

void ServerLevel::sendParticles(const ParticleType* type, double x, double y, double z, int count, double xDist, double yDist, double zDist, double maxSpeed, arrayWithLength<int> params, double xAux, double yAux, double zAux)
{
    boost::shared_ptr<ClientboundLevelParticlesPacket> packet(
        new ClientboundLevelParticlesPacket(type, false, (float)x, (float)y, (float)z, (float)xDist, (float)yDist, (float)zDist, (float)maxSpeed, count, params, (float)xAux, (float)yAux, (float)zAux));
    for (size_t i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = boost::dynamic_pointer_cast<ServerPlayer>(mPlayers[i]);
        sendParticles(player, false, packet);
    }
}

void ServerLevel::sendParticles(boost::shared_ptr<ServerPlayer> player, const ParticleType* type, bool longDistance, double x, double y, double z, int count, double xDist, double yDist, double zDist, double maxSpeed, arrayWithLength<int> params)
{
    boost::shared_ptr<ClientboundLevelParticlesPacket> packet(
        new ClientboundLevelParticlesPacket(type, longDistance, (float)x, (float)y, (float)z, (float)xDist, (float)yDist, (float)zDist, (float)maxSpeed, count, params));
    sendParticles(player, longDistance, packet);
}

void ServerLevel::getEntityByUUID(uint32_t first, uint32_t second)
{
    (void)first; (void)second;
}

uint64_t ServerLevel::findNearestMapFeature(std::wstring* first, BlockPos* second, bool third)
{
    if (!chunkSource || !first || !second)
        return 0;
    return static_cast<ServerChunkCache*>(chunkSource)->findNearestMapFeature(this, *first, *second, third) ? 1U : 0U;
}

boost::shared_ptr<ServerPlayer> ServerLevel::getPlayer(const PlayerUID& playerUID) const
{
    return server->getPlayers()->getPlayer(playerUID);
}

void ServerLevel::queueSendBlockUpdate(int32_t first, int32_t second, int32_t third, BlockState* fourth, BlockState* fifth)
{
    QueuedSendBlockData* update = new QueuedSendBlockData;
    update->x = first; update->y = second; update->z = third;
    update->oldState = fourth; update->newState = fifth;
    queuedBlockUpdates.push_back(update);
}

bool ServerLevel::atBlockEntityLimit(int32_t first, uint32_t second)
{
    (void)first;
    return blockEntityCount >= static_cast<int32_t>(second);
}

uint32_t ServerLevel::setBlock(BlockPos* first, BlockState* second, int32_t third, bool fourth)
{
    return first && second && Level::setBlock(*first, second, third, fourth) ? 1U : 0U;
}

bool ServerLevel::addEntity(boost::shared_ptr<Entity> first)
{
    if (!first || atEntityLimit(first))
        return false;
    entityAdded(first);
    return true;
}

bool ServerLevel::atEntityLimit(boost::shared_ptr<Entity> first)
{
    return !first || !first->isAlive();
}

bool ServerLevel::newPrimedTntAllowed()
{
    return static_cast<uint32_t>(blockEventCount) < 30U;
}

bool ServerLevel::newFallingBlockAllowed()
{
    return static_cast<uint32_t>(villageSiegeTick) < 20U;
}

bool ServerLevel::explodeNextEnderCrystal()
{
    uint32_t count = static_cast<uint32_t>(enderCrystalCount);
    ++enderCrystalCount;
    return count < 30U;
}

void ServerLevel::flagEntitiesToBeRemoved(uint32_t* first, bool* second)
{
}

Scoreboard* ServerLevel::GetScoreboard()
{
    return serverScoreboard;
}

ServerScoreboard* ServerLevel::GetServerScoreboard()
{
    return serverScoreboard;
}

uint64_t ServerLevel::invalidateChunksOutsideArea(int32_t first, int32_t second, int32_t third, int32_t fourth)
{
    if (chunkSource != 0)
        static_cast<ServerChunkCache*>(chunkSource)->invalidateChunksOutsideArea(first, second, third, fourth);
    return 0;
}

void ServerLevel::blockEntityAddedExtra(boost::shared_ptr<BlockEntity> first)
{
    if (first)
        ++blockEntityCount;
}

void ServerLevel::blockEntityRemovedExtra(boost::shared_ptr<BlockEntity> first)
{
    if (first && blockEntityCount > 0)
        --blockEntityCount;
}
