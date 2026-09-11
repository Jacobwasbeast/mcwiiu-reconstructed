#include "game/world/generation/MultiPlayerLevel.hpp"

#include <algorithm>
#include <cmath>

#include "game/client/audio/MinecartSoundInstance.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/FireworksParticles__FireworksStarter.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Options.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/data/MockedLevelStorage.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelSettings.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ClientChunkCache.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

MultiPlayerLevel::MultiPlayerLevel(boost::shared_ptr<LevelStorage> levelStorage,
                                   ClientPacketListener* connection, LevelSettings* settings,
                                   int dimension, const Difficulty* difficulty, int seaLevel)
    : Level(levelStorage, new LevelData(settings, L"MpServer"),
            DimensionType::getById(dimension)->create(), true),
      mAnimateTickReady(true), mAnimateTickX(0), mAnimateTickY(0),
      mChunkTickX(0), mChunkTickZ(0), mClientChunkCache(NULL),
      mMinecraft(NULL), mClientScoreboard(NULL), mAmbientSoundTime(0)
{
    MultiPlayerLevelInit(connection, settings, difficulty, seaLevel);
}

MultiPlayerLevel::MultiPlayerLevel(ClientPacketListener* connection, LevelSettings* settings,
                                   int dimension, const Difficulty* difficulty, int seaLevel)
    : Level(boost::shared_ptr<LevelStorage>(new MockedLevelStorage()),
            new LevelData(settings, L"MpServer"), DimensionType::getById(dimension)->create(), true),
      mAnimateTickReady(true), mAnimateTickX(0), mAnimateTickY(0),
      mChunkTickX(0), mChunkTickZ(0), mClientChunkCache(NULL),
      mMinecraft(NULL), mClientScoreboard(NULL), mAmbientSoundTime(0)
{
    MultiPlayerLevelInit(connection, settings, difficulty, seaLevel);
}

MultiPlayerLevel::~MultiPlayerLevel()
{
    delete mClientScoreboard;
}

void MultiPlayerLevel::removeClientPacketListener(ClientPacketListener* connection, bool sendDisconnect)
{
    if (sendDisconnect)
    {
        boost::shared_ptr<Packet> packet(new DisconnectPacket(DisconnectPacket::eDisconnectReason_Quit));
        connection->sendAndDisconnect(packet);
    }

    std::vector<ClientPacketListener*>::iterator found =
        std::find(mConnections.begin(), mConnections.end(), connection);
    if (found != mConnections.end())
        mConnections.erase(found);
}

ClientScoreboard* MultiPlayerLevel::GetClientScoreboard()
{
    return mClientScoreboard;
}

void MultiPlayerLevel::animateTick(int x, int y, int z)
{
    int sectionX = x >> 3;
    int sectionY = y >> 3;
    int sectionZ = z >> 3;
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dz = -1; dz <= 1; ++dz)
            {
                int sx = sectionX + dx;
                int sy = sectionY + dy;
                int sz = sectionZ + dz;
                if ((unsigned int)(sx + 54) < 108 && (unsigned int)sy < 32 &&
                    (unsigned int)(sz + 54) < 108)
                    mAnimatedSections.insert((sx << 20) | ((sy & 255) << 12) | (sz & 4095));
            }
        }
    }
}

void MultiPlayerLevel::animateTickDoWork()
{
    Random random;
    for (int pass = 0; pass < 16 && !mAnimatedSections.empty(); ++pass)
    {
        boost::unordered_set<int>::const_iterator section = mAnimatedSections.begin();
        for (; section != mAnimatedSections.end(); ++section)
        {
            int packed = *section;
            int sectionX = packed >> 20;
            int sectionY = (packed >> 12) & 255;
            int sectionZ = (packed << 20) >> 20;
            BlockPos pos(sectionX * 8 + random.nextInt(8),
                         sectionY * 8 + random.nextInt(8),
                         sectionZ * 8 + random.nextInt(8));
            const BlockState* state = getBlockState(pos);
            state->getBlock()->animateTick(state, this, pos, &random);
        }
    }
    mAnimatedSections.clear();
}

void MultiPlayerLevel::removeAllPendingEntityRemovals()
{
    EntitySet pending = mPendingEntityAdditions;
    mPendingEntityAdditions.clear();
    for (EntitySet::iterator it = pending.begin(); it != pending.end(); ++it)
    {
        if (mTrackedEntities.find(*it) != mTrackedEntities.end())
            Level::removeEntity(*it);
    }
}

void MultiPlayerLevel::playLocalSound(const BlockPos& pos, const SoundEvent* sound,
                                      SoundSource::ESoundSource source, float volume, float pitch,
                                      bool distanceDelay, float range)
{
    playLocalSound(pos.x + 0.5, pos.y + 0.5, pos.z + 0.5,
                   sound, source, volume, pitch, distanceDelay, range);
}

void MultiPlayerLevel::MultiPlayerLevelInit(ClientPacketListener* connection, LevelSettings*,
                                             const Difficulty* difficulty, int seaLevel)
{
    mMinecraft = Minecraft::GetInstance();
    if (seaLevel != -1)
        setSeaLevel(seaLevel);
    createChunkSource();
    if (connection != NULL)
        mConnections.push_back(connection);
    getLevelData()->setDifficulty(difficulty);
    mAnimateTickReady = true;
    mAnimateTickX = 0;
    mAnimateTickY = 0;
    mChunkTickX = 0;
    mChunkTickZ = 0;
    mClientScoreboard = new ClientScoreboard();
    updateSkyBrightness();
    prepareWeather();
    mAmbientSoundTime = mRandom->nextInt(12000);
}

void MultiPlayerLevel::tick()
{
    setGameTime(getGameTime() + 1);
    if (getGameRules()->getBoolean(8))
        setDayTime(getDayTime() + 1);

    int added = 0;
    while (!mPendingEntityAdditions.empty() && added < 10)
    {
        EntitySet::iterator it = mPendingEntityAdditions.begin();
        boost::shared_ptr<Entity> entity = *it;
        mPendingEntityAdditions.erase(it);
        if (mTrackedEntities.find(entity) != mTrackedEntities.end())
            Level::addEntity(entity);
        ++added;
    }

    std::vector<ClientPacketListener*> connections(mConnections);
    for (std::vector<ClientPacketListener*>::iterator it = connections.begin();
         it != connections.end(); ++it)
        (*it)->tick();

    for (std::vector<ResetInfo>::iterator it = mResetInfos.begin(); it != mResetInfos.end();)
    {
        --it->delay;
        if (it->delay == 0)
        {
            setBlockAndUpdate(it->pos, it->state);
            sendBlockUpdated(it->pos, NULL, NULL, 3, true);
            it = mResetInfos.erase(it);
        }
        else
            ++it;
    }

    mClientChunkCache->tick();
    Level::tick();
}

void MultiPlayerLevel::clearResetRegion(int minX, int minY, int minZ,
                                        int maxX, int maxY, int maxZ)
{
    for (unsigned int i = 0; i < mResetInfos.size(); ++i)
    {
        std::vector<ResetInfo>::iterator it = mResetInfos.begin() + i;
        if (minX <= it->pos.x && minY <= it->pos.y && minZ <= it->pos.z &&
            it->pos.x <= maxX && it->pos.y <= maxY && it->pos.z <= maxZ)
        {
            mResetInfos.erase(it);
            --i;
        }
    }
}

ChunkSource* MultiPlayerLevel::createChunkSource()
{
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) ||
        CGameNetworkManager::sInstance.IsHost())
        mClientChunkCache = new ClientChunkCache(this);
    else
        mClientChunkCache = new ClientChunkCache(
            this, getLevelStorage()->createChunkStorage(mDimension));
    return mClientChunkCache;
}

ChunkSource* MultiPlayerLevel::getChunkSource()
{
    return Level::getChunkSource();
}

bool MultiPlayerLevel::hasChunk(int x, int z, bool allowEmpty)
{
    return allowEmpty || !getChunkSource()->getOrCreateChunk(x, z, false)->isEmpty();
}

bool MultiPlayerLevel::hasChunksAt(int minX, int minY, int minZ,
                                   int maxX, int maxY, int maxZ, bool allowEmpty)
{
    return allowEmpty || Level::hasChunksAt(minX, minY, minZ, maxX, maxY, maxZ, allowEmpty);
}

LevelChunk* MultiPlayerLevel::getChunkIfHasChunk(int x, int z, bool allowEmpty)
{
    return hasChunk(x, z, allowEmpty) ? getChunk(x, z) : NULL;
}

void MultiPlayerLevel::validateSpawn()
{
}

void MultiPlayerLevel::tickBlocks()
{
    if (mAmbientSoundTime > 0)
        --mAmbientSoundTime;

    mChunksToPoll.clear();

    PIXBeginNamedEvent(0.0f, "Ticking blocks (multiplayer)");
    PIXBeginNamedEvent(0.0f, "buildAndPrepareChunksToPoll");
    Level::tickBlocks();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Ticking client side blocks");
    for (int i = 0; i < mChunksToPoll.end(); ++i)
    {
        ChunkPos pos = mChunksToPoll.get(i);
        LevelChunk* chunk = getChunk(pos.x, pos.z);
        tickClientSideBlocks(pos.x * 16, pos.z * 16, chunk);
    }
    PIXEndNamedEvent();
    PIXEndNamedEvent();
}

void MultiPlayerLevel::ReloadChunk(int x, int z,
                                   const std::vector<LevelChunk::BlockChange>* changes, bool replace)
{
    mClientChunkCache->queueForReLoad(x, z, changes, replace);
}

void MultiPlayerLevel::setChunkVisible(int x, int z, bool visible, bool force)
{
    if (visible)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Any, true) &&
            !CGameNetworkManager::sInstance.IsHost() && !hasChunk(x, z, false))
            ReloadChunk(x, z, new std::vector<LevelChunk::BlockChange>(), false);
        mClientChunkCache->create(x, z);
    }
    else
    {
        mClientChunkCache->drop(x, z, force);
        BlockPos from(x * 16, 0, z * 16);
        BlockPos to(x * 16 + 15, 256, z * 16 + 15);
        setBlocksDirty(from, to);
    }
}

bool MultiPlayerLevel::addEntity(boost::shared_ptr<Entity> entity)
{
    bool added = Level::addEntity(entity);
    mTrackedEntities.insert(entity);
    if (!added)
        mPendingEntityAdditions.insert(entity);
    else if ((entity->GetType() & 0x0a800000ULL) == 0x0a800000ULL)
        static_cast<ConsoleSoundEngine*>(mMinecraft->mSoundEngine)->play(
            new MinecartSoundInstance(boost::static_pointer_cast<Minecart>(entity)));
    return added;
}

void MultiPlayerLevel::removeEntity(boost::shared_ptr<Entity> entity)
{
    EntitySet::iterator pending = mPendingEntityAdditions.find(entity);
    if (pending != mPendingEntityAdditions.end())
        mPendingEntityAdditions.erase(pending);
    Level::removeEntity(entity);
    mTrackedEntities.erase(entity);
}

void MultiPlayerLevel::entityAdded(boost::shared_ptr<Entity> entity)
{
    Level::entityAdded(entity);
    EntitySet::iterator pending = mPendingEntityAdditions.find(entity);
    if (pending != mPendingEntityAdditions.end())
        mPendingEntityAdditions.erase(pending);
}

void MultiPlayerLevel::entityRemoved(boost::shared_ptr<Entity> entity)
{
    Level::entityRemoved(entity);
    if (mTrackedEntities.find(entity) != mTrackedEntities.end())
    {
        entity->stopCurrentLerp();
        mPendingEntityAdditions.insert(entity);
    }
}

void MultiPlayerLevel::putEntity(int id, boost::shared_ptr<Entity> entity)
{
    boost::shared_ptr<Entity> oldEntity = getEntity(id);
    if (oldEntity)
        removeEntity(oldEntity);
    mTrackedEntities.insert(entity);
    entity->setId(id);
    if (!addEntity(entity))
        mPendingEntityAdditions.insert(entity);
    mEntitiesById[id] = entity;
}

boost::shared_ptr<Entity> MultiPlayerLevel::getEntity(int id)
{
    for (int i = 0; i < 4; ++i)
    {
        if (mMinecraft->mLocalPlayers[i] && mMinecraft->mLocalPlayers[i]->getId() == id)
            return mMinecraft->mLocalPlayers[i];
    }

    EntityMap::iterator found = mEntitiesById.find(id);
    if (found == mEntitiesById.end())
        return boost::shared_ptr<Entity>();
    return found->second;
}

boost::shared_ptr<Entity> MultiPlayerLevel::removeEntity(int id)
{
    boost::shared_ptr<Entity> entity;
    EntityMap::iterator found = mEntitiesById.find(id);
    if (found != mEntitiesById.end())
    {
        entity = found->second;
        mEntitiesById.erase(found);
        mTrackedEntities.erase(entity);
        removeEntity(entity);
    }
    return entity;
}

void MultiPlayerLevel::removeEntities(std::vector<boost::shared_ptr<Entity> >* entities)
{
    std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
    while (it != entities->end())
    {
        boost::shared_ptr<Entity> entity = *it;
        if ((entity->GetType() & 0x0c800000ULL) == 0x0c800000ULL)
        {
            it = entities->erase(it);
        }
        else
        {
            EntitySet::iterator pending = mPendingEntityAdditions.find(entity);
            if (pending != mPendingEntityAdditions.end())
                mPendingEntityAdditions.erase(pending);
            mTrackedEntities.erase(entity);
            ++it;
        }
    }
    Level::removeEntities(entities);
}

bool MultiPlayerLevel::setBlock(const BlockPos& pos, const BlockState* state,
                                int flags, bool forceUpdate)
{
    return Level::setBlock(pos, state, flags, forceUpdate);
}

bool MultiPlayerLevel::doSetBlockAndData(const BlockPos& pos, const BlockState* state,
                                         const BlockState* extraState)
{
    clearResetRegion(pos.x, pos.y, pos.z, pos.x, pos.y, pos.z);
    const BlockState* oldState = getBlockState(pos);

    bool liquidFlowChange =
        (oldState->getBlock() == Blocks::FLOWING_WATER && state->getBlock() == Blocks::WATER) ||
        (oldState->getBlock() == Blocks::WATER && state->getBlock() == Blocks::FLOWING_WATER) ||
        (oldState->getBlock() == Blocks::flowing_lava && state->getBlock() == Blocks::lava) ||
        (oldState->getBlock() == Blocks::lava && state->getBlock() == Blocks::lava) ||
        (oldState->getBlock() == Blocks::lava && state->getBlock() == Blocks::flowing_lava);

    bool changed = false;
    if (extraState != NULL)
        changed = setExtraBlock(pos, extraState, 3, false);
    changed = setBlockAndUpdate(pos, state) || changed;

    if (!changed && !CGameNetworkManager::sInstance.IsHost())
        return false;

    if (CGameNetworkManager::sInstance.IsHost() && !liquidFlowChange)
    {
        sendBlockUpdated(pos, oldState, state, 3, true);
        blockUpdated(pos, state->getBlock(), true);
    }
    return true;
}

void MultiPlayerLevel::disconnect(bool save)
{
    for (std::vector<ClientPacketListener*>::iterator it = mConnections.begin();
         it != mConnections.end(); ++it)
    {
        if (save)
        {
            boost::shared_ptr<Packet> packet(new DisconnectPacket(DisconnectPacket::eDisconnectReason_Quit));
            (*it)->sendAndDisconnect(packet);
        }
        else
            (*it)->close();
    }
}

SoundInstance* MultiPlayerLevel::makeSoundUpdater(boost::shared_ptr<Minecart>)
{
    return NULL;
}

void MultiPlayerLevel::tickWeather(bool)
{
}

int MultiPlayerLevel::getChunkPollRange()
{
    return mMinecraft->mOptions->mViewDistance;
}

void MultiPlayerLevel::tickClientSideBlocks(int x, int z, LevelChunk* chunk)
{
    Level::tickClientSideBlocks(x, z, chunk);
    if (mAmbientSoundTime != 0)
        return;

    mRandValue = mRandValue * 3 + mAddend;
    int value = mRandValue >> 2;
    int localX = value & 15;
    int localY = (value >> 16) & 127;
    int localZ = (value >> 8) & 15;
    BlockPos pos(localX, localY, localZ);
    if (chunk->getBlock(pos)->mMaterial != Material::AIR)
        return;
    if (getDaytimeRawBrightness(pos) > mRandom->nextInt(8))
        return;
    if (chunk->getBrightness(LightLayer::SKY, pos) > 0)
        return;

    double soundX = localX + x + 0.5;
    double soundY = localY + 0.5;
    double soundZ = localZ + z + 0.5;
    boost::shared_ptr<Player> player = getNearestPlayer(soundX, soundY, soundZ, 8.0, false, -1.0);
    Player* nearest = player.get();
    if (nearest != NULL && nearest->distanceToSqr(soundX, soundY, soundZ) > 4.0)
    {
        playLocalSound(soundX, soundY, soundZ, SoundEvents::AMBIENT_CAVE,
                       SoundSource::eSoundSource_Ambient, 0.7f,
                       mRandom->nextFloat() * 0.2f + 0.8f, false, 16.0f);
        mAmbientSoundTime = mRandom->nextInt(12000) + 6000;
    }
}

void MultiPlayerLevel::playSound(boost::shared_ptr<Player> player, double x, double y, double z,
                                 const SoundEvent* sound, SoundSource::ESoundSource source,
                                 float volume, float pitch, float range)
{
    if ((mMinecraft->mLocalPlayers[0] && player == mMinecraft->mLocalPlayers[0]) ||
        (mMinecraft->mLocalPlayers[1] && player == mMinecraft->mLocalPlayers[1]) ||
        (mMinecraft->mLocalPlayers[2] && player == mMinecraft->mLocalPlayers[2]) ||
        (mMinecraft->mLocalPlayers[3] && player == mMinecraft->mLocalPlayers[3]))
        playLocalSound(x, y, z, sound, source, volume, pitch, false, range);
}

void MultiPlayerLevel::playLocalSound(double x, double y, double z, const SoundEvent* sound,
                                      SoundSource::ESoundSource source, float volume, float pitch,
                                      bool distanceDelay, float range)
{
    if (volume > 1.0f)
        range *= volume;
    boost::shared_ptr<Player> player = getNearestPlayer(x, y, z, range, false, -1.0);
    if (!player)
        return;
    float distanceSquared = (float)player->distanceToSqr(x, y, z);
    int delay = 0;
    if (distanceDelay && distanceSquared > 100.0f)
        delay = (int)std::floor(std::sqrt(distanceSquared) / 40.0f * 20.0f + 0.5f);
    mMinecraft->mSoundEngine->schedule(sound, source, (float)x, (float)y, (float)z,
                                       volume, pitch, delay);
}

void MultiPlayerLevel::createFireworks(double x, double y, double z,
                                       double xa, double ya, double za, CompoundTag* fireworks)
{
    FireworksParticles::FireworksStarter* particle =
        new FireworksParticles::FireworksStarter(this, x, y, z, xa, ya, za,
                                                 mMinecraft->mParticleEngine, fireworks);
    mMinecraft->mParticleEngine->add(particle);
}

void MultiPlayerLevel::sendPacketToServer(boost::shared_ptr<Packet>)
{
}

void MultiPlayerLevel::setDayTime(long long time)
{
    Level::setDayTime(time);
}

void MultiPlayerLevel::dataReceivedForChunk(int x, int z)
{
    mClientChunkCache->dataReceived(x, z);
}

void MultiPlayerLevel::removeUnusedBlockEntitiesInRegion(int minX, int minY, int minZ,
                                                          int maxX, int maxY, int maxZ)
{
    std::vector<BlockPos> removed;
    for (int chunkX = minX >> 4; chunkX <= (maxX - 1) >> 4; ++chunkX)
    {
        for (int chunkZ = minZ >> 4; chunkZ <= (maxZ - 1) >> 4; ++chunkZ)
        {
            LevelChunk* chunk = getChunk(chunkX, chunkZ);
            if (chunk == NULL)
                continue;
            LevelChunk::BlockEntityMap* blockEntities = chunk->getBlockEntities();
            for (LevelChunk::BlockEntityMap::iterator it = blockEntities->begin();
                 it != blockEntities->end(); ++it)
            {
                const BlockPos& pos = it->first;
                if (pos.x >= minX && pos.x < maxX && pos.y >= minY && pos.y < maxY &&
                    pos.z >= minZ && pos.z < maxZ && it->second->isRemoved())
                    removed.push_back(pos);
            }
        }
    }
    for (std::vector<BlockPos>::iterator it = removed.begin(); it != removed.end(); ++it)
        removeBlockEntity(*it);
}

Scoreboard* MultiPlayerLevel::GetScoreboard()
{
    return mClientScoreboard;
}

void MultiPlayerLevel::DropChunksOutsideArea(int minX, int minZ, int maxX, int maxZ, bool force)
{
    mClientChunkCache->dropOutsideArea(minX, minZ, maxX, maxZ, force);
}

void MultiPlayerLevel::StopReloadChunk(int x, int z)
{
    mClientChunkCache->unqueueForReLoad(x, z);
}

void MultiPlayerLevel::MustQueueBlockEntityDataPackets()
{
    mClientChunkCache->mustQueueBlockEntityDataPackets();
}

bool MultiPlayerLevel::shouldQueueBlockEntityDataPackets()
{
    return mClientChunkCache != 0 && mClientChunkCache->mustQueueBlockEntityDataPackets();
}

void MultiPlayerLevel::QueueBlockEntityDataPacket(
    const boost::shared_ptr<ClientboundBlockEntityDataPacket>& packet)
{
    mClientChunkCache->queueBlockEntityDataPacket(packet);
}

void MultiPlayerLevel::BlockUntilSafeToChangeSaveData(bool block)
{
    mClientChunkCache->BlockUntilSafeToChangeSaveData(block);
}
