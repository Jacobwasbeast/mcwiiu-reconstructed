#include "game/server/ServerLevel.hpp"
#include "game/world/LevelSettings.hpp"
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
#include "game/network/ProgressListener.hpp"
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
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/DLCPack.hpp"
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
#include "game/world/chunk/PlayerChunkMap.hpp"
#include "game/world/chunk/MinigamePlayerChunkMap.hpp"
#include "game/entity/mob/NaturalMobSpawner.hpp"
#include "game/world/generation/VillageSiege.hpp"
#include "game/core/PortalForcer.hpp"
#include "game/network/ServerLevelListener.hpp"
#include "game/core/CriticalSection.hpp"
#include "platform/wiiu/C4JEventArrayImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/server/DerivedServerLevel.hpp"
#include "game/data/SavedDataStorage.hpp"
#include "game/core/LootTables.hpp"
#include "game/network/ClientboundLevelParticlesPacket.hpp"
#include "game/network/ClientboundEntityEventPacket.hpp"
#include "game/world/generation/Villages.hpp"
#include "game/world/generation/BonusChestFeature.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/LevelData.hpp"
#include "game/entity/mob/EntityTracker.hpp"

#include <cstring>
#include <typeinfo>

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
    : Level(storage, data, DimensionType::getById(dimension)->create(), false), server(server), tracker(0), chunkMap(0), emptyTime(0), tickNextTickSet(11), blockEventCount(0), allPlayersSleeping(false), updatingBlocks(false), portalForcer(0), naturalMobSpawner(0), villageSiege(0), blockEntityCount(0), villageSiegeTick(0), enderCrystalCount(0), mField0x2cc(false), saving(false), serverScoreboard(0), weatherTick(0), primedTntCount(0), fallingBlockCount(0), fireworksRocketCount(0), beaconCount(0)
{
    InitializeCriticalSection(&tickNextTickMutex);
    InitializeCriticalSection(&entityLimitMutex);
    mDimension->init(this);
    mChunkSource = createChunkSource();
    server->setLevel(dimension, this);
    addListener(new ServerLevelListener(server, this));
    tracker = new EntityTracker(this);

    const int viewDistance = server->getPlayers()->getViewDistance();
    if (Minecraft::InMiniGame(static_cast<EMiniGameId>(15), true))
        chunkMap = new MinigamePlayerChunkMap(this, dimension, viewDistance);
    else
        chunkMap = new PlayerChunkMap(this, dimension, viewDistance);

    naturalMobSpawner = 0;
    portalForcer = new PortalForcer(this);
    villageSiege = new VillageSiege(this);
    updateSkyBrightness();
    prepareWeather();
}

ServerLevel::~ServerLevel()
{
    delete portalForcer;
    delete naturalMobSpawner;
    delete villageSiege;
    delete serverScoreboard;
    delete tracker;
    delete chunkMap;
}

bool ServerLevel::AllPlayersAreSleeping()
{
    return allPlayersSleeping;
}

EntityTracker* ServerLevel::getTracker()
{
    return tracker;
}

ChunkSource* ServerLevel::getChunkSource()
{
    return Level::getChunkSource();
}

std::vector<boost::shared_ptr<BlockEntity> >* ServerLevel::getBlockEntitiesInRegion(int32_t first, int32_t second, int32_t third, int32_t fourth, int32_t fifth, int32_t sixth)
{
    std::vector<boost::shared_ptr<BlockEntity> >* result = new std::vector<boost::shared_ptr<BlockEntity> >();
    for (std::vector<boost::shared_ptr<BlockEntity> >::iterator it = mBlockEntities.begin();
         it != mBlockEntities.end(); ++it)
    {
        boost::shared_ptr<BlockEntity> entity = *it;
        if (!entity)
            continue;
        BlockPos pos = entity->getBlockPos();
        if (first <= pos.x && second <= pos.y && third <= pos.z
            && pos.x < fourth && pos.y < fifth && pos.z < sixth)
        {
            result->push_back(entity);
        }
    }
    return result;
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
    setSavedDataStorage(new SavedDataStorage(mLevelStorage.get()));

    std::wstring id = Villages::getFileId(mDimension);
    SavedDataStorage* storage = getSavedDataStorage();
    boost::shared_ptr<SavedData> saved = storage->get(typeid(Villages), id);
    boost::shared_ptr<Villages> villages = boost::static_pointer_cast<Villages>(saved);
    if (!villages)
    {
        villages.reset(new Villages(this));
        storage->set(id, villages);
    }
    else
    {
        villages->setLevel(this);
    }
    mVillages = villages;
    serverScoreboard = new ServerScoreboard(server);
    mLootTables = new LootTables();
}

uint32_t ServerLevel::allPlayersAreSleeping()
{
    if (!allPlayersSleeping || mIsClientSide)
        return 0;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if ((*it)->isSpectator())
            continue;
        if (!(*it)->isSleepingLongEnough())
            return 0;
    }
    return 1;
}

void ServerLevel::stopWeather()
{
    mLevelData->setRainTime(0);
    mLevelData->setRaining(false);
    mLevelData->setThunderTime(0);
    mLevelData->setThundering(false);
}

uint64_t ServerLevel::awakenAllPlayers()
{
    allPlayersSleeping = false;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if ((*it)->isSleeping())
            (*it)->stopSleepInBed(false, false, true);
    }
    if (getGameRules()->getBoolean(18))
        stopWeather();
    return 1;
}

void ServerLevel::save(bool first, ProgressListener* second, bool third)
{
    ChunkSource* source = getChunkSource();
    if (!source->shouldSave())
        return;

    if (Minecraft::GetInstance()->isClientSide())
        return;

    if (second != NULL)
        second->progressStage(third ? static_cast<int>(0x5015737c) : static_cast<int>(0xf5938f13u));

    PIXBeginNamedEvent(1.035f, "Saving level data");
    ServerLevel* dispatchLevel = this;
    dispatchLevel->saveLevelData();
    PIXEndNamedEvent();

    if (second != NULL)
        second->progressStage(static_cast<int>(0xd0f458bd));

    source->save(first, second);
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
    std::vector<Biome::MobSpawnerData*> mobs = getChunkSource()->getMobsAt(first, *third);
    if (mobs.empty())
        return false;

    for (std::vector<Biome::MobSpawnerData*>::iterator it = mobs.begin(); it != mobs.end(); ++it)
    {
        if (*it == second)
            return true;
    }
    return false;
}

void ServerLevel::updateSleepingPlayerList()
{
    mField0x2cc = false;
    if (mPlayers.empty())
    {
        allPlayersSleeping = false;
        return;
    }

    uint32_t awakeCount = 0;
    uint32_t sleepCount = 0;
    allPlayersSleeping = false;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if ((*it)->isSpectator())
            awakeCount++;
        else if ((*it)->isSleeping())
        {
            sleepCount++;
            mField0x2cc = true;
        }
    }

    if (sleepCount > 0 && (static_cast<uint32_t>(mPlayers.size()) - awakeCount) <= sleepCount)
        allPlayersSleeping = true;
}

void ServerLevel::validateSpawn()
{
    if (mLevelData->getYSpawn() < 1)
        mLevelData->setYSpawn(getSeaLevel() + 1);

    int spawnX = mLevelData->getXSpawn();
    int spawnZ = mLevelData->getZSpawn();
    const BlockState* state = getTopBlockState(BlockPos(spawnX, 0, spawnZ));
    Block* block = state != NULL ? state->getBlock() : NULL;
    if (block == NULL)
    {
        int attempts = 0;
        do
        {
            spawnX += mRandom->nextInt(8) - mRandom->nextInt(8);
            spawnZ += mRandom->nextInt(8) - mRandom->nextInt(8);
            ++attempts;
            if (attempts == 10000)
                break;
            state = getTopBlockState(BlockPos(spawnX, 0, spawnZ));
            block = state != NULL ? state->getBlock() : NULL;
        }
        while (block == NULL);
    }

    mLevelData->setXSpawn(spawnX);
    mLevelData->setZSpawn(spawnZ);
}

bool ServerLevel::hasChunk(int32_t first, int32_t second, bool third)
{
    ChunkSource* source = getChunkSource();
    return source != 0 && source->hasChunk(first, second);
}

LevelChunk* ServerLevel::getChunkIfHasChunk(int32_t first, int32_t second, bool third)
{
    return getChunkSource()->getChunkIfHasChunk(first, second);
}

BlockPos ServerLevel::findLightingTargetAround(const BlockPos& first)
{
    return getTopRainBlockPos(first);
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
                lightningPosition = findLightingTargetAround(lightningPosition);
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
    if (mPlayers.empty())
    {
        if (emptyTime++ > 299)
            return;
    }
    else
    {
        resetEmptyTime();
    }

    mDimension->tick();
    Level::tickEntities();
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
            for (std::set<TickNextTickData, TickNextTickDataKeyCompare>::iterator it = tickNextTickList.begin();
                 it != tickNextTickList.end(); ++it)
            {
                TickNextTickData data = *it;
                if (data.pos.x < box->x0 || box->x1 <= data.pos.x
                    || data.pos.z < box->z0 || box->z1 <= data.pos.z)
                {
                    continue;
                }
                if (removeTicks)
                {
                    tickNextTickSet.erase(data);
                }
                ticks->push_back(data);
            }
        }
        else
        {
            if (removeTicks)
            {
                for (std::vector<TickNextTickData>::iterator it = toBeTicked.begin(); it != toBeTicked.end();)
                {
                    TickNextTickData data = *it;
                    if (data.pos.x < box->x0 || box->x1 <= data.pos.x
                        || data.pos.z < box->z0 || box->z1 <= data.pos.z)
                    {
                        ++it;
                        continue;
                    }
                    std::pair<std::set<TickNextTickData, TickNextTickDataKeyCompare>::iterator,
                              std::set<TickNextTickData, TickNextTickDataKeyCompare>::iterator> range =
                        tickNextTickList.equal_range(data);
                    tickNextTickList.erase(range.first, range.second);
                    it = toBeTicked.erase(it);
                    ticks->push_back(data);
                }
            }
            else
            {
                for (std::vector<TickNextTickData>::iterator it = toBeTicked.begin(); it != toBeTicked.end(); ++it)
                {
                    TickNextTickData data = *it;
                    if (data.pos.x >= box->x0 && data.pos.x < box->x1
                        && data.pos.z >= box->z0 && data.pos.z < box->z1)
                    {
                        ticks->push_back(data);
                    }
                }
            }
        }
    }

    LeaveCriticalSection(&tickNextTickMutex);
    return ticks;
}

bool ServerLevel::isAnimalsAllowed()
{
    return server != NULL && server->isAnimals();
}

void ServerLevel::isNpcsAllowed()
{
    if (server != NULL)
        server->isNpcsEnabled();
}

void ServerLevel::tick(boost::shared_ptr<Entity> first, bool second)
{
    if (!isAnimalsAllowed())
    {
        if ((first->GetType() & 0xd680000) == 0xd680000
            || (first->GetType() & 0xd502000) == 0xd502000
            || (first->GetType() & 0xd602000) == 0xd602000)
        {
            first->remove();
        }
    }

    Level::tick(first, second);
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
    if (!mLevelData->isInitialized())
    {
        Level::initializeLevel(first);
        mLevelData->setInitialized(true);
    }
}

void ServerLevel::setupDebugLevel()
{
    mLevelData->setGenerateMapFeatures(false);
    mLevelData->setAllowCommands(true);
    mLevelData->setRaining(false);
    mLevelData->setThundering(false);
    mLevelData->setClearWeatherTime(1000000000);
    mLevelData->setDayTime(6000);
    mLevelData->setGameType(GameType::SPECTATOR);
    mLevelData->setHardcore(false);
    mLevelData->setDifficulty(Difficulty::PEACEFUL);
    getGameRules()->set(8, false);
}

uint64_t ServerLevel::generateBonusItemsNearSpawn()
{
    int spawnX = mLevelData->getXSpawn();
    int spawnZ = mLevelData->getZSpawn();
    for (int dx = -20; dx <= 20; ++dx)
    {
        for (int dz = -20; dz <= 20; ++dz)
        {
            BlockPos surface = getTopSolidBlockPos(BlockPos(spawnX + dx, 0, spawnZ + dz));
            BlockPos below = surface.below();
            if (getBlockId(below.x, below.y, below.z) != 0)
                continue;

            boost::shared_ptr<BlockEntity> blockEntity = getBlockEntity(below);
            boost::shared_ptr<ChestBlockEntity> chest =
                boost::dynamic_pointer_cast<ChestBlockEntity>(blockEntity);
            if (chest != NULL && chest->getLootTable() != NULL)
                return 0;
        }
    }

    BonusChestFeature* feature = new BonusChestFeature();
    int attempt = 0;
    for (int tries = 8; tries > 0; --tries)
    {
        int x = spawnX + mRandom->nextInt(6) - mRandom->nextInt(6);
        int z = spawnZ + mRandom->nextInt(6) - mRandom->nextInt(6);
        BlockPos placePos = getTopSolidBlockPos(BlockPos(x, 0, z)).above();
        if (feature->place(this, *mRandom, placePos, attempt == 15))
            break;
        if (feature->place(this, *mRandom, placePos, attempt == 14))
            break;
        attempt += 2;
    }
    delete feature;
    return 0;
}

void ServerLevel::setInitialSpawn(LevelSettings* first)
{
    if (!mDimension->mayRespawn())
    {
        mLevelData->setSpawn(BlockPos(0, mDimension->getSpawnYPosition(), 0));
        return;
    }

    mFindingSpawn = true;

    Random random(getSeed());
    const int halfSize = mDimension->getXZSize() * 8;
    const int minBound = -halfSize;
    const int maxBound = halfSize - 1;

    int spawnX = 8;
    int spawnY = mDimension->getSpawnYPosition();
    int spawnZ = 8;
    int attempts = 0;
    while (!mDimension->isValidSpawn(spawnX, spawnZ))
    {
        spawnX += random.nextInt(64) - random.nextInt(64);
        if (maxBound < spawnX)
            spawnX = 0;
        if (spawnX < minBound)
            spawnX = 0;

        spawnZ += random.nextInt(64) - random.nextInt(64);
        if (maxBound < spawnZ)
            spawnZ = 0;
        if (spawnZ < minBound)
            spawnZ = 0;

        ++attempts;
        if (attempts == 1000)
            break;
    }

    mLevelData->setSpawn(BlockPos(spawnX, spawnY, spawnZ));

    if (first->hasStartingBonusItems())
        generateBonusItemsNearSpawn();

    mFindingSpawn = false;
}

uint64_t ServerLevel::Suspend()
{
    saving = true;
    return 1;
}

void ServerLevel::saveToDisc(ProgressListener* first, bool second)
{
    if (Minecraft::GetInstance()->isClientSide())
        return;

    TexturePack* selected = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    if (selected != TexturePackRepository::defaultTexturePack &&
        !selected->getDLCPack()->hasPurchasedPack())
        return;

    if (first != NULL)
        first->progressStage(static_cast<int>(0xaebf0574u));

    mLevelStorage->flushSaveFile(second);
}

bool ServerLevel::removeDuplicateAndPendingEntity(const boost::shared_ptr<Entity>& entity)
{
    if (entity->mRemoved)
        return false;
    const std::wstring uuid = entity->getUUID();
    boost::unordered_map<std::wstring, boost::shared_ptr<Entity> >::iterator existing = entitiesByUuid.find(uuid);
    if (existing != entitiesByUuid.end())
    {
        boost::shared_ptr<Entity> duplicate = existing->second;
        std::vector<boost::shared_ptr<Entity> >::iterator pending =
            std::find(mEntitiesToRemove.begin(), mEntitiesToRemove.end(), duplicate);
        if (pending != mEntitiesToRemove.end())
            mEntitiesToRemove.erase(pending);
        else if ((entity->GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER)
            return false;
        Level::removeEntityImmediately(duplicate);
    }
    return true;
}

void ServerLevel::addEntities(std::vector<boost::shared_ptr<Entity> >* first)
{
    EnterCriticalSection(&mEntityMutex);

    bool sawSpecialType = false;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = first->begin(); it != first->end(); ++it)
    {
        if (removeDuplicateAndPendingEntity(*it))
        {
            mEntities.push_back(*it);
            entityAdded(*it);
            if ((*it)->GetType() == 0xd000005ULL)
                sawSpecialType = true;
        }
    }

    if (sawSpecialType)
    {
        bool keptFirst = false;
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin(); it != mEntities.end(); ++it)
        {
            if ((*it)->GetType() == 0xd000005ULL)
            {
                if (keptFirst)
                    (*it)->remove();
                else
                    keptFirst = true;
            }
        }
    }

    LeaveCriticalSection(&mEntityMutex);
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

void ServerLevel::entityAddedExtra(boost::shared_ptr<Entity> entity)
{
    const unsigned long long type = entity->GetType();
    std::list<boost::shared_ptr<Entity> >* category = NULL;
    if (type == 0x0800000aULL)
        category = &itemEntities;
    else if ((type & 0x0a080000ULL) == 0x0a080000ULL)
        category = &hangingEntities;
    else if ((type & 0x8a020000ULL) == 0x8a020000ULL)
        category = &arrowEntities;
    else if (type == 0x08000003ULL)
        category = &experienceOrbEntities;
    else if (type == ENTITY_TYPE_PRIMED_TNT)
    {
        EnterCriticalSection(&entityLimitMutex);
        ++primedTntCount;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (type == 0x08000009ULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        ++fallingBlockCount;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (type == 0x88000005ULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        ++fireworksRocketCount;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (type == 0x0800000fULL)
        category = &areaEffectCloudEntities;

    if (category != NULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        category->push_back(entity);
        LeaveCriticalSection(&entityLimitMutex);
    }
}

void ServerLevel::entityAdded(boost::shared_ptr<Entity> entity)
{
    Level::entityAdded(entity);
    mEntityMap[entity->getId()] = entity;
    entitiesByUuid[entity->getUUID()] = entity;
    std::vector<boost::shared_ptr<Entity> >* parts = entity->getSubEntities();
    if (parts != NULL)
        for (std::vector<boost::shared_ptr<Entity> >::iterator part = parts->begin(); part != parts->end(); ++part)
            mEntityMap[(*part)->getId()] = *part;
    entityAddedExtra(entity);
}

void ServerLevel::entityRemovedExtra(boost::shared_ptr<Entity> entity)
{
    const unsigned long long type = entity->GetType();
    std::list<boost::shared_ptr<Entity> >* category = NULL;
    if (type == 0x0800000aULL)
        category = &itemEntities;
    else if ((type & 0x0a080000ULL) == 0x0a080000ULL)
        category = &hangingEntities;
    else if ((type & 0x8a020000ULL) == 0x8a020000ULL)
        category = &arrowEntities;
    else if (type == 0x08000003ULL)
        category = &experienceOrbEntities;
    else if (type == ENTITY_TYPE_PRIMED_TNT)
    {
        EnterCriticalSection(&entityLimitMutex);
        --primedTntCount;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (type == 0x08000009ULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        --fallingBlockCount;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (type == 0x88000005ULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        --fireworksRocketCount;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (type == 0x0800000fULL)
        category = &areaEffectCloudEntities;

    if (category != NULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        std::list<boost::shared_ptr<Entity> >::iterator existing =
            std::find(category->begin(), category->end(), entity);
        if (existing != category->end())
            category->erase(existing);
        LeaveCriticalSection(&entityLimitMutex);
    }
}

void ServerLevel::entityRemoved(boost::shared_ptr<Entity> entity)
{
    Level::entityRemoved(entity);
    mEntityMap.erase(entity->getId());
    entitiesByUuid.erase(entity->getUUID());
    std::vector<boost::shared_ptr<Entity> >* parts = entity->getSubEntities();
    if (parts != NULL)
        for (std::vector<boost::shared_ptr<Entity> >::iterator part = parts->begin(); part != parts->end(); ++part)
            mEntityMap.erase((*part)->getId());
    entityRemovedExtra(entity);
}

uint64_t ServerLevel::getEntity(uint32_t first, uint32_t second)
{
    (void)second;
    uint64_t result = 0;
    new (&result) boost::shared_ptr<Entity>(Level::getEntity(static_cast<int>(first)));
    return result;
}

bool ServerLevel::addGlobalEntity(boost::shared_ptr<Entity> first)
{
    return Level::addGlobalEntity(first);
}

void ServerLevel::broadcastEntityEvent(boost::shared_ptr<Entity> first, uint8_t second, int32_t third)
{
    boost::shared_ptr<Packet> packet(new ClientboundEntityEventPacket(first, second, third));
    getTracker()->broadcastAndSend(first, packet);
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
    (void)first;
    const long long oldDayTime = getDayTime();
    const unsigned int oldDayTimeLow = static_cast<unsigned int>(oldDayTime);
    const int oldDayTimeHigh = static_cast<int>(oldDayTime >> 32);
    const unsigned int deltaLow = third - oldDayTimeLow;
    const int deltaHigh = second - (oldDayTimeHigh + (third < oldDayTimeLow ? 1 : 0));

    std::vector<TickNextTickData> adjustedTicks;
    EnterCriticalSection(&tickNextTickMutex);
    for (std::set<TickNextTickData, TickNextTickDataKeyCompare>::iterator it = tickNextTickList.begin();
         it != tickNextTickList.end(); ++it)
    {
        TickNextTickData data = *it;
        const unsigned int triggerLow = static_cast<unsigned int>(data.triggerTick);
        const int triggerHigh = static_cast<int>(data.triggerTick >> 32);
        const unsigned int newTriggerLow = triggerLow + deltaLow;
        const int newTriggerHigh = triggerHigh + deltaHigh +
            (newTriggerLow < triggerLow ? 1 : 0);
        data.triggerTick = (static_cast<long long>(newTriggerHigh) << 32) |
            static_cast<unsigned int>(newTriggerLow);
        adjustedTicks.push_back(data);
    }
    tickNextTickList.clear();
    tickNextTickSet.clear();
    for (std::vector<TickNextTickData>::iterator it = adjustedTicks.begin(); it != adjustedTicks.end(); ++it)
    {
        tickNextTickList.insert(*it);
        tickNextTickSet.insert(*it);
    }
    LeaveCriticalSection(&tickNextTickMutex);

    const long long newGameTime = (static_cast<long long>(second) << 32) |
        static_cast<unsigned int>(third);
    setGameTime(newGameTime);
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
    boost::shared_ptr<Entity>* out =
        reinterpret_cast<boost::shared_ptr<Entity>*>(static_cast<uintptr_t>(first));
    const std::wstring* uuid =
        reinterpret_cast<const std::wstring*>(static_cast<uintptr_t>(second));
    if (out == NULL || uuid == NULL)
        return;

    boost::unordered_map<std::wstring, boost::shared_ptr<Entity> >::iterator it =
        entitiesByUuid.find(*uuid);
    *out = it != entitiesByUuid.end() ? it->second : boost::shared_ptr<Entity>();
}

uint64_t ServerLevel::findNearestMapFeature(std::wstring* first, BlockPos* second, bool third)
{
    ChunkSource* source = getChunkSource();
    if (!source || !first || !second)
        return 0;
    return source->findNearestMapFeature(this, *first, *second, third) ? 1U : 0U;
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

bool ServerLevel::atBlockEntityLimit(Block* first)
{
    bool result = false;

    if (first == Blocks::wall_banner || first == Blocks::standing_banner)
    {
        EnterCriticalSection(&mBlockEntityMutex);
        result = blockEntityCount > 99;
        LeaveCriticalSection(&mBlockEntityMutex);
    }

    if (first == Blocks::beacon)
    {
        EnterCriticalSection(&mBlockEntityMutex);
        result = beaconCount > 249;
        LeaveCriticalSection(&mBlockEntityMutex);
    }

    return result;
}

uint32_t ServerLevel::setBlock(BlockPos* first, BlockState* second, int32_t third, bool fourth)
{
    return first && second && Level::setBlock(*first, second, third, fourth) ? 1U : 0U;
}

bool ServerLevel::addEntity(boost::shared_ptr<Entity> entity)
{
    if (!removeDuplicateAndPendingEntity(entity))
        return false;

    const unsigned long long type = entity->GetType();
    std::list<boost::shared_ptr<Entity> >* category = NULL;
    unsigned int limit = 0;
    if (type == 0x0800000aULL)
    {
        category = &itemEntities;
        limit = 200;
    }
    else if ((type & 0x0a080000ULL) == 0x0a080000ULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        const bool full = hangingEntities.size() >= 400;
        LeaveCriticalSection(&entityLimitMutex);
        if (full)
            return false;
    }
    else if ((type & 0x8a020000ULL) == 0x8a020000ULL)
    {
        category = &arrowEntities;
        limit = 200;
    }
    else if (type == 0x08000003ULL)
    {
        category = &experienceOrbEntities;
        limit = 50;
    }
    else if (type == 0x0800000fULL)
    {
        category = &areaEffectCloudEntities;
        limit = 20;
    }
    if (category != NULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        if (category->size() >= limit)
        {
            boost::shared_ptr<Entity> oldest = category->front();
            Level::removeEntityImmediately(oldest);
        }
        LeaveCriticalSection(&entityLimitMutex);
    }
    return Level::addEntity(entity);
}

bool ServerLevel::atEntityLimit(boost::shared_ptr<Entity> entity)
{
    const unsigned long long type = entity->GetType();
    bool full = false;
    if (type == 0x0800000aULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        full = itemEntities.size() >= 200;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if ((type & 0x0a080000ULL) == 0x0a080000ULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        full = hangingEntities.size() >= 400;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if ((type & 0x8a020000ULL) == 0x8a020000ULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        full = arrowEntities.size() >= 200;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (type == ENTITY_TYPE_PRIMED_TNT)
    {
        EnterCriticalSection(&entityLimitMutex);
        full = primedTntCount >= 20;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (type == 0x08000003ULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        full = experienceOrbEntities.size() >= 50;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (type == 0x88000005ULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        full = fireworksRocketCount >= 20;
        LeaveCriticalSection(&entityLimitMutex);
    }
    else if (type == 0x0800000fULL)
    {
        EnterCriticalSection(&entityLimitMutex);
        full = areaEffectCloudEntities.size() >= 20;
        LeaveCriticalSection(&entityLimitMutex);
    }
    return full;
}

bool ServerLevel::newPrimedTntAllowed()
{
    EnterCriticalSection(&entityLimitMutex);
    bool allowed = primedTntCount < 20;
    LeaveCriticalSection(&entityLimitMutex);
    return allowed;
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
    if (chunkMap == NULL)
        return;
    chunkMap->flagEntitiesToBeRemoved(reinterpret_cast<unsigned int*>(first), second);
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
    ChunkSource* source = getChunkSource();
    if (source != 0)
        static_cast<ServerChunkCache*>(source)->invalidateChunksOutsideArea(first, second, third, fourth);
    return 0;
}

void ServerLevel::blockEntityAddedExtra(boost::shared_ptr<BlockEntity> first)
{
    if (first->GetType() == 0x100020cULL)
    {
        EnterCriticalSection(&mBlockEntityMutex);
        ++blockEntityCount;
        LeaveCriticalSection(&mBlockEntityMutex);
    }

    if (first->GetType() == 0x1e00204ULL)
    {
        EnterCriticalSection(&mBlockEntityMutex);
        ++beaconCount;
        LeaveCriticalSection(&mBlockEntityMutex);
    }
}

void ServerLevel::blockEntityRemovedExtra(boost::shared_ptr<BlockEntity> first)
{
    if (first->GetType() == 0x100020cULL)
    {
        EnterCriticalSection(&mBlockEntityMutex);
        --blockEntityCount;
        LeaveCriticalSection(&mBlockEntityMutex);
    }

    if (first->GetType() == 0x1e00204ULL)
    {
        EnterCriticalSection(&mBlockEntityMutex);
        --beaconCount;
        LeaveCriticalSection(&mBlockEntityMutex);
    }
}
