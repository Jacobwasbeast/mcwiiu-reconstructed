#include "game/server/MinecraftServer.hpp"

#include <cstdlib>
#include <ctime>
#include <algorithm>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/EServerSetting.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GameRuleManager.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Settings.hpp"
#include "game/core/Socket.hpp"
#include "game/core/System.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientboundSetTimePacket.hpp"
#include "game/network/Packet.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/DerivedServerLevel.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/network/ServerConnection.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/UpdateProgressPacket.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelSettings.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRules.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "vendor/_NetworkGameInitData.hpp"
#include "vendor/DiskSaveBuffer.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/ConsoleSaveFileOriginal.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/data/McRegionLevelStorage.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/core/FileEntry.hpp"
#include "game/core/Compression.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/data/SparseLightStorage.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/LevelChunk.hpp"

extern CConsoleMinecraftApp gApp;

extern void Sleep(unsigned int milliseconds);
extern void instructionSynchronize();
extern void PIXAddNamedCounter(float colour, const char* name, ...);

MinecraftServer* gMinecraftServer = 0;

static bool sHaltForceFlag = false;
static bool sHaltRequested = false;
extern CStorage* gStorage;
extern CTelemetryManager gTelemetryManager;

MinecraftServer::MinecraftServer()
    : mConnection(0), mSettings(0), mPlayers(0), mLoginComparisonValue(0), tickCount(0), running(true), levelLoaded(false), shuttingDown(false), saving(false), networkTick(0), progressNameLength(0), progressNameCapacity(0), progressValue(0), commandDispatcher(0), consoleInputIndex(0), consoleInputCount(0), animals(true), npcsEnabled(true), pvpAllowed(true), flightAllowed(false), serverFlags(0), statusMessageLength(0), statusMessageCapacity(0), maxBuildHeight(256), difficulty(0), levelType(0), defaultGameType(GameType::SURVIVAL), gameModeId(0), playerIdleTimeout(0), progressRenderer(0), levelStorage(0), postProcessState(0), postProcessThread(0), serverPaused(false), forceGameType(false), suspending(false), stopping(false), lastServerTime(0), postProcessing(false), suspended(false), shutdownFlags(0), mMasterGameMode(0)
{
    unsigned int i;
    for (i = 0; i < 8; ++i)
    {
        schematicFiles[i] = 0;
        fillTools[i] = 0;
    }
    consoleInputs.reserve(4);
    postProcessRequests.reserve(8);
}

MinecraftServer::~MinecraftServer()
{
    delete[] mLevels.data;
    mLevels.data = 0;
    mLevels.length = 0;
    consoleInputs.clear();
    postProcessRequests.clear();
    mPlayers = 0;
    mConnection = 0;
    mSettings = 0;
}

bool MinecraftServer::IsServerPaused() const { return serverPaused; }
void MinecraftServer::setFlightAllowed(bool allowed) { flightAllowed = allowed; }
void MinecraftServer::setPlayers(PlayerList* value) { mPlayers = value; }
void MinecraftServer::setAnimals(bool value) { animals = value; }
void MinecraftServer::setNpcsEnabled(bool value) { npcsEnabled = value; }
void MinecraftServer::setPvpAllowed(bool value) { pvpAllowed = value; }
void MinecraftServer::setMaxBuildHeight(int value) { maxBuildHeight = value; }
int MinecraftServer::getMaxBuildHeight() { return maxBuildHeight; }
bool MinecraftServer::isHardcore() { return false; }
bool MinecraftServer::getSpawnMonsters() { return true; }

void MinecraftServer::setLevelDifficulty(ServerLevel* level, const Difficulty* difficulty)
{
    if (level == 0)
        return;

    LevelData* levelData = level->getLevelData();
    if (levelData->isHardcore())
    {
        levelData->setDifficulty(Difficulty::HARD);
        level->setSpawnSettings(true, true);
    }
    else
    {
        levelData->setDifficulty(difficulty);
        level->setSpawnSettings(getSpawnMonsters(), animals);
    }
}

void MinecraftServer::setDifficulty(const Difficulty* difficulty)
{
    unsigned int i;
    for (i = 0; i < mLevels.length; ++i)
        if (mLevels.data[i] != 0)
            setLevelDifficulty(mLevels.data[i], difficulty);
}

int MinecraftServer::runPostUpdate(void* argument)
{
    MinecraftServer* server = static_cast<MinecraftServer*>(argument);

    ShutdownManager::HasStarted(ShutdownManager::eThreadId_ServerPostUpdate);
    Entity::useSmallIds();
    IntCache::CreateNewThreadStorage();
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    phys::Vec2::CreateNewThreadStorage();
    Compression::UseDefaultThreadStorage();
    Level::enableLightingCache();
    Block::CreateNewThreadStorage();
    LevelChunk::CreateNewThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_ServerPostUpdate);

    bool stoppingFlag;
    do
    {
        EnterCriticalSection(&server->postProcessLock);
        if (server->postProcessRequests.empty())
        {
            LeaveCriticalSection(&server->postProcessLock);
            Sleep(1);
            stoppingFlag = server->stopping;
        }
        else
        {
            postProcessRequest request = server->postProcessRequests.back();
            server->postProcessRequests.pop_back();
            LeaveCriticalSection(&server->postProcessLock);
            PIXBeginNamedEvent(0.0f, "Post processing");
            LevelChunk* chunk = request.source->getChunk(request.x, request.z);
            if (chunk != 0)
                chunk->doPostProcess(request.source, request.generator);
            PIXEndNamedEvent();
            Sleep(1);
            stoppingFlag = server->stopping;
        }
    }
    while (!stoppingFlag && ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ServerPostUpdate));

    while (true)
    {
        EnterCriticalSection(&server->postProcessLock);
        if (server->postProcessRequests.empty()
            || !ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ServerPostUpdate))
            break;

        postProcessRequest request = server->postProcessRequests.back();
        server->postProcessRequests.pop_back();
        LeaveCriticalSection(&server->postProcessLock);
        LevelChunk* chunk = request.source->getChunk(request.x, request.z);
        if (chunk != 0)
            chunk->doPostProcess(request.source, request.generator);
    }
    LeaveCriticalSection(&server->postProcessLock);

    Block::ReleaseThreadStorage();
    IntCache::ReleaseThreadStorage();
    AABB::ReleaseThreadStorage();
    Vec3::ReleaseThreadStorage();
    phys::Vec2::ReleaseThreadStorage();
    Level::destroyLightingCache();
    ElementStorage::ReleaseThreadStorage();
    LevelChunk::ReleaseThreadStorage();
    ShutdownManager::HasFinished(ShutdownManager::eThreadId_ServerPostUpdate);
    instructionSynchronize();
    return 0;
}
void MinecraftServer::postProcessTerminate(ProgressRenderer* renderer)
{
    EnterCriticalSection(&postProcessLock);
    unsigned int initialCount = static_cast<unsigned int>(postProcessRequests.size());
    LeaveCriticalSection(&postProcessLock);

    while (postProcessThread != 0
        && postProcessThread->WaitForCompletion(50) == 0x102)
    {
        EnterCriticalSection(&postProcessLock);
        unsigned int currentCount = static_cast<unsigned int>(postProcessRequests.size());
        LeaveCriticalSection(&postProcessLock);

        if (initialCount != 0)
            renderer->progressStagePercentage(
                static_cast<int>((initialCount - currentCount) * 100 / initialCount));
        CompressedBlockStorage::tick();
        SparseLightStorage::tick();
        SparseDataStorage::tick();
    }

    if (postProcessThread != 0)
        delete postProcessThread;
    postProcessThread = 0;
    DeleteCriticalSection(&postProcessLock);
}
ServerLevel* MinecraftServer::getLevel(int dimension)
{
    unsigned int index = dimension == -1 ? 1 : dimension == 1 ? 2 : 0;
    return index < mLevels.length ? mLevels.data[index] : 0;
}
void MinecraftServer::saveGameRules()
{
    unsigned char* data = 0;
    unsigned int size = 0;
    gApp.mGameRuleManager.saveGameRules(&data, &size);
    if (data == 0 || size == 0)
        return;

    ServerLevel* level = getLevel(0);
    if (level == 0) {
        delete[] data;
        return;
    }
    boost::shared_ptr<LevelStorage> storage = level->getLevelStorage();
    if (!storage) {
        delete[] data;
        return;
    }
    ConsoleSaveFile* saveFile = storage->getSaveFile();
    if (saveFile == 0) {
        delete[] data;
        return;
    }
    FileEntry* entry = saveFile->createFile(ConsoleSavePath(L"requiredGameRules.grf"));
    if (entry != 0) {
        int high = 0;
        unsigned int written = 0;
        saveFile->setFilePointer(entry, 0, &high, 0);
        saveFile->writeFile(entry, data, size, &written);
        saveFile->closeHandle(entry);
    }
    delete[] data;
}
void MinecraftServer::endProgress()
{
    progressName.clear();
    progressValue = 0;
}
bool MinecraftServer::loadLevel(LevelStorageSource*, const std::wstring& name,
                                long long seed, LevelType* type, _NetworkGameInitData* initData)
{
    ProgressRenderer* progressRenderer = Minecraft::GetInstance()->mProgressRenderer;
    mLevels.resize(3);
    mLevels.data[0] = 0;

    std::wstring saveName;
    void* saveData = 0;
    unsigned int saveSize = 0;
    ESavePlatform platform = eSavePlatform_WIIU;
    if (initData != 0 && initData->m_value2 != 0)
    {
        DiskSaveBuffer* buffer = initData->m_value2;
        saveData = buffer->data;
        saveSize = buffer->length;
        saveName = buffer->name;
        platform = static_cast<ESavePlatform>(initData->m_value8);
    }

    ConsoleSaveFile* saveFile = new ConsoleSaveFileOriginal(
        saveName, saveData, saveSize, false, platform);
    boost::shared_ptr<LevelStorage> storage(new McRegionLevelStorage(
        saveFile, File(L"."), name, true, Minecraft::GetInstance()->getFixerUpper()));
    LevelData* levelData = storage->prepareLevel();
    LevelSettings* levelSettings = 0;

    if (levelData == 0)
    {
        const int requestedGameType = initData == 0
            ? static_cast<int>(defaultGameType->getId()) : initData->m_value9;
        const GameType* gameType = LevelSettings::validateGameType(requestedGameType);
        levelSettings = new LevelSettings(seed, gameType,
            gApp.GetGameHostOption(static_cast<eGameHostOption>(8)) != 0,
            isHardcore(), true, type,
            initData == 0 ? 0 : initData->m_value7,
            initData == 0 ? 0 : initData->m_mode,
            0x80, true,
            initData == 0 ? 0 : initData->m_flag1,
            initData == 0 ? 0 : initData->m_value11,
            initData == 0 ? 0 : initData->m_value12);
        if (gApp.GetGameHostOption(static_cast<eGameHostOption>(9)) != 0)
            levelSettings->enableStartingBonusItems();
        if (initData != 0 && initData->m_value10 != 0)
            levelSettings->setLevelTypeOptions(initData->m_value10);
        levelData = new LevelData(levelSettings, name);
    }
    else
        levelSettings = new LevelSettings(levelData);

    ServerLevel* level = new ServerLevel(this, storage, levelData, 0);
    level->postConstruct();
    setLevel(0, level);
    level->initializeLevel(levelSettings);
    levelData->setGameType(levelSettings->getGameType());
    DerivedServerLevel* nether = new DerivedServerLevel(this, storage, -1, level);
    nether->postConstruct();
    setLevel(-1, nether);
    DerivedServerLevel* end = new DerivedServerLevel(this, storage, 1, level);
    end->postConstruct();
    setLevel(1, end);

    if (mPlayers != 0)
    {
        mPlayers->setLevel(arrayWithLength<ServerLevel*>(mLevels.data, mLevels.length));
        setDifficulty(Difficulty::byId(gApp.GetGameHostOption(static_cast<eGameHostOption>(0))));
    }

    InitializeCriticalSection(&postProcessLock);
    stopping = false;
    postProcessThread = new C4JThreadImpl(
        &MinecraftServer::runPostUpdate, this, "Post processing", 0x40000);
    postProcessThread->SetPriority(2);
    postProcessThread->Run();

    unsigned long long preparationStart = System::processTimeInMilliSecs();
    unsigned long long lastStorageTick = preparationStart;
    const int totalChunks = 3 * 25 * 25;
    int preparedChunks = 0;
    for (unsigned int dimensionIndex = 0; dimensionIndex < mLevels.length; ++dimensionIndex)
    {
        ServerLevel* currentLevel = mLevels.data[dimensionIndex];
        if (currentLevel == 0)
            continue;
        BlockPos spawn = currentLevel->getSharedSpawnPos();
        ChunkSource* chunkSource = currentLevel->getChunkSource();
        for (int x = -196; x <= 196 && running && CGameNetworkManager::sInstance.IsInSession(); x += 16)
        {
            for (int z = -196; z <= 196 && running && CGameNetworkManager::sInstance.IsInSession(); z += 16)
            {
                if (chunkSource != 0)
                {
                    ServerChunkCache* serverChunkSource = static_cast<ServerChunkCache*>(chunkSource);
                    serverChunkSource->create((spawn.x + x) >> 4, (spawn.z + z) >> 4, true);
                }
                ++preparedChunks;
                if (progressRenderer != 0)
                    progressRenderer->progressStagePercentage((preparedChunks * 100) / totalChunks);
                unsigned long long now = System::processTimeInMilliSecs();
                if (now - lastStorageTick >= 50)
                {
                    CompressedBlockStorage::tick();
                    SparseLightStorage::tick();
                    SparseDataStorage::tick();
                    lastStorageTick = now;
                }
            }
        }
    }
    stopping = true;
    postProcessTerminate(progressRenderer);
    level->validateSpawn();
    delete levelSettings;
    levelStorage = 0;
    (void)initData;
    return true;
}

bool MinecraftServer::initServer(long long seed, _NetworkGameInitData* initData, unsigned int, bool dedicated)
{
    mSettings = new Settings();
    setFlightAllowed(true);

    mConnection = new ServerConnection(this);
    Socket::Initialise(mConnection);

    while (LevelGenerationOptions* generationOptions = gApp.getLevelGenerationOptions())
    {
        if (generationOptions->hasLoadedData())
            break;
        Sleep(1);
    }

    if (LevelGenerationOptions* generationOptions = gApp.getLevelGenerationOptions())
        generationOptions->ready();

    LevelRuleset* gameRuleDefinitions = gApp.getGameRuleDefinitions();
    LevelGenerationOptions* generationOptions = gApp.getLevelGenerationOptions();
    if (gameRuleDefinitions == 0 || initData == 0 || initData->m_value5 == 0
        || generationOptions == 0 || !generationOptions->isGameModeLobby())
    {
        mMasterGameMode = 0;
        mPlayers = new PlayerList(this, 0);
        setPlayers(mPlayers);
        setAnimals(mSettings->getBoolean(eServerSetting_SpawnAnimals, true));
        setNpcsEnabled(mSettings->getBoolean(eServerSetting_SpawnNpcs, true));
        setPvpAllowed(mSettings->getBoolean(
            eServerSetting_Pvp,
            gApp.GetGameHostOption(static_cast<eGameHostOption>(0xb)) != 0));
    }
    else
    {
        mMasterGameMode = new MasterGameMode(&initData->m_ruleGroups);
        mMasterGameMode->SetupServerSettings(mSettings);
        mPlayers = new PlayerList(this, mMasterGameMode);
        setPlayers(mPlayers);
        setAnimals(mSettings->getBoolean(eServerSetting_SpawnAnimals, true));
        setNpcsEnabled(mSettings->getBoolean(eServerSetting_SpawnNpcs, true));
        setPvpAllowed(mSettings->getBoolean(
            eServerSetting_Pvp,
            gApp.GetGameHostOption(static_cast<eGameHostOption>(0xb)) != 0));
    }

    std::wstring levelName = mSettings->getString(eServerSetting_LevelName, L"world");

    std::wstring levelTypeDefault = L"flat";
    if ((generationOptions == 0 || !generationOptions->getuseFlatWorld())
        && gApp.GetGameHostOption(static_cast<eGameHostOption>(7)) == 0)
    {
        levelTypeDefault = L"default";
    }
    std::wstring levelTypeName = mSettings->getString(eServerSetting_LevelType, levelTypeDefault);

    LevelType* resolvedLevelType = LevelType::getLevelType(levelTypeName);
    LevelType* seedLevelType = 0;
    if (initData != 0)
        seedLevelType = reinterpret_cast<LevelType*>(initData->m_value13);
    if (seedLevelType == 0)
    {
        seedLevelType = resolvedLevelType;
        if (seedLevelType == 0)
            seedLevelType = LevelType::DEFAULT;
    }

    ProgressRenderer* progressRenderer = Minecraft::GetInstance()->mProgressRenderer;
    if (progressRenderer != 0)
        progressRenderer->progressStage(static_cast<int>(0xf990f518u));

    if (dedicated)
        BiomeSource::findSeed(seedLevelType);

    int buildHeight = mSettings->getInt(eServerSetting_MaxBuildHeight, 0x100);
    setMaxBuildHeight(buildHeight);
    buildHeight = getMaxBuildHeight() + 8;
    setMaxBuildHeight(((buildHeight >> 4) + ((buildHeight < 0 && (buildHeight & 0xf) != 0) ? 1 : 0)) * 0x10);
    setMaxBuildHeight(Mth::clamp(getMaxBuildHeight(), 0x40, 0x100));

    if (initData != 0 && initData->m_flag2 != 0)
    {
        typedef int (*FnE4)(void*);
        typedef void (*FnC4)(void*, int*);

        int storageProbe = 0;
        void* storageVTable = *reinterpret_cast<void**>(gStorage);
        reinterpret_cast<FnC4>(*reinterpret_cast<void**>(reinterpret_cast<char*>(storageVTable) + 0xc4))(gStorage, &storageProbe);

        void* profileVTable = *reinterpret_cast<void**>(gProfile);
        int profileFirst = reinterpret_cast<FnE4>(*reinterpret_cast<void**>(reinterpret_cast<char*>(profileVTable) + 0xe4))(gProfile);
        int profileSlot = reinterpret_cast<FnE4>(*reinterpret_cast<void**>(reinterpret_cast<char*>(profileVTable) + 0xe4))(gProfile);

        unsigned int difficultySetting = GameSettings::GetGameSettings(profileSlot, static_cast<eGameSetting>(3));
        unsigned int localPlayerCount = gApp.GetLocalPlayerCount();
        int onlinePlayerCount = CGameNetworkManager::sInstance.GetOnlinePlayerCount();

        gTelemetryManager.RecordLevelResume(profileFirst, static_cast<ESen_FriendOrMatch>(1),
            static_cast<ESen_CompeteOrCoop>(3), difficultySetting, localPlayerCount, onlinePlayerCount, 0);
    }

    levelType = resolvedLevelType;
    levelLoaded = loadLevel(0, levelName, seed, levelType, initData);

    if (initData != 0 && initData->m_value2 != 0)
    {
        std::free(initData->m_value2->data);
        initData->m_value2->length = 0;
        initData->m_value2->data = 0;
    }

    CGameNetworkManager::sInstance.ServerReady();
    return levelLoaded;
}

void MinecraftServer::saveAllChunks()
{
    unsigned int i;
    for (i = 0; i < mLevels.length; ++i)
        if (mLevels.data[i] != 0)
            mLevels.data[i]->saveToDisc(0, true);
}

void MinecraftServer::stopServer(bool save)
{
    Minecraft::GetInstance()->mGameRenderer->DisableUpdateThread(0);
    mConnection->stop();

    if (!sHaltForceFlag)
    {
        void* profileVTable = *reinterpret_cast<void**>(gProfile);
        typedef int (*Fn0xE4)(void*);
        int call1Result = reinterpret_cast<Fn0xE4>(*reinterpret_cast<void**>(reinterpret_cast<char*>(profileVTable) + 0xe4))(gProfile);
        typedef int (*Fn0x24)(void*, int);
        int call2Result = reinterpret_cast<Fn0x24>(*reinterpret_cast<void**>(reinterpret_cast<char*>(profileVTable) + 0x24))(gProfile, call1Result);

        if (call2Result != 0 && postProcessing)
        {
            typedef int (*Fn0x11C)(void*);
            int call3Result = reinterpret_cast<Fn0x11C>(*reinterpret_cast<void**>(reinterpret_cast<char*>(*reinterpret_cast<void**>(gProfile)) + 0x11c))(gProfile);
            if (call3Result != 0)
            {
                void* storageVTable = *reinterpret_cast<void**>(gStorage);
                typedef int (*Fn0x64)(void*);
                int call4Result = reinterpret_cast<Fn0x64>(*reinterpret_cast<void**>(reinterpret_cast<char*>(storageVTable) + 100))(gStorage);
                if (call4Result == 0 && save)
                {
                    if (mPlayers != 0)
                    {
                        mPlayers->saveAll(reinterpret_cast<ProgressListener*>(Minecraft::GetInstance()->mProgressRenderer), true);
                        mPlayers->removeAll();
                    }
                    saveAllChunks();
                    saveGameRules();
                    gApp.mGameRuleManager.unloadCurrentGameRules();
                    ServerLevel* level0 = mLevels.data[0];
                    if (level0 != 0)
                        level0->saveToDisc(reinterpret_cast<ProgressListener*>(Minecraft::GetInstance()->mProgressRenderer), false);
                }
            }
        }
    }

    sHaltForceFlag = false;
    sHaltRequested = false;

    if (mMasterGameMode != 0)
    {
        void* secondary = *reinterpret_cast<void**>(reinterpret_cast<char*>(mMasterGameMode) + 0x124);
        typedef void (*FnSecondary0x24)(MasterGameMode*);
        reinterpret_cast<FnSecondary0x24>(*reinterpret_cast<void**>(reinterpret_cast<char*>(secondary) + 0x24))(mMasterGameMode);
        if (mMasterGameMode != 0)
        {
            secondary = *reinterpret_cast<void**>(reinterpret_cast<char*>(mMasterGameMode) + 0x124);
            typedef void (*FnSecondary0xC)(void*, int);
            reinterpret_cast<FnSecondary0xC>(*reinterpret_cast<void**>(reinterpret_cast<char*>(secondary) + 0xc))(mMasterGameMode, 3);
        }
        mMasterGameMode = 0;
    }

    delete mConnection;
    mConnection = 0;
    delete mPlayers;
    unsigned int levelCount = mLevels.length;
    mPlayers = 0;
    for (unsigned int i = 0; i < levelCount; ++i)
    {
        if (mLevels.data[i] != 0)
        {
            delete mLevels.data[i];
            mLevels.data[i] = 0;
        }
    }

    delete mSettings;
    mSettings = 0;

    SparseLightStorage::tick();
    SparseLightStorage::tick();
    SparseLightStorage::tick();

    CGameNetworkManager::sInstance.ServerStopped();
}
PlayerList* MinecraftServer::getPlayers() { return mPlayers; }
void MinecraftServer::setDefaultGameMode(const GameType* value) { defaultGameType = value; }
const GameType* MinecraftServer::getDefaultGameType() { return defaultGameType; }
ServerConnection* MinecraftServer::getConnection() { return mConnection; }
bool MinecraftServer::isAnimals() { return animals; }
bool MinecraftServer::isNpcsEnabled() { return npcsEnabled; }
bool MinecraftServer::isPvpAllowed() { return pvpAllowed; }
bool MinecraftServer::isFlightAllowed() { return flightAllowed; }
bool MinecraftServer::isCommandBlockEnabled() { return false; }
bool MinecraftServer::isNetherEnabled() { return true; }
CommandDispatcher* MinecraftServer::getCommandDispatcher() { return commandDispatcher; }
int MinecraftServer::getSpawnProtectionRadius() { return 16; }

bool MinecraftServer::isUnderSpawnProtection(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player)
{
    if (level == 0 || mPlayers == 0 || player.get() == 0)
        return false;
    boost::shared_ptr<ServerPlayer> serverPlayer = boost::dynamic_pointer_cast<ServerPlayer>(player);
    if (mPlayers->isOp(serverPlayer))
        return false;
    BlockPos spawn = level->getSharedSpawnPos();
    int dx = Mth::abs(pos.x - spawn.x);
    int dz = Mth::abs(pos.z - spawn.z);
    return (dx > dz ? dx : dz) <= getSpawnProtectionRadius();
}

bool MinecraftServer::getForceGameType() { return forceGameType; }
unsigned long long MinecraftServer::getCurrentTimeMillis()
{
    return System::processTimeInMilliSecs();
}
int MinecraftServer::getSpawnRadius(ServerLevel* level) { return level == 0 ? 10 : level->getGameRules()->getInt(15); }
void MinecraftServer::halt() { running = false; }
void MinecraftServer::fjSetServerPaused(bool value) { serverPaused = value; }
void MinecraftServer::tick()
{
    AABB::resetPool();
    Vec3::resetPool();

    ++tickCount;

    const float pixColor = 3.0f;
    for (unsigned int levelIndex = 0; levelIndex < mLevels.length; ++levelIndex)
    {
        ServerLevel* level = mLevels.data[levelIndex];
        if (level == 0)
            continue;

        if (tickCount % 20 == 0)
        {
            GameRules* gameRules = level->getGameRules();
            if (gameRules->getBoolean(8) || level->ShouldForceTimeUpdate())
            {
                const long long gameTime = level->getGameTime();
                const long long dayTime = level->getDayTime();
                const bool daylightCycle = gameRules->getBoolean(8);
                boost::shared_ptr<Packet> packet(new ClientboundSetTimePacket(
                    gameTime, dayTime, daylightCycle));
                mPlayers->broadcastAll(
                    packet,
                    level->mDimension->getType()->getId());
            }
        }

        PIXBeginNamedEvent(pixColor, "Level tick %d", levelIndex);
        level->tick();
        PIXEndNamedEvent();

        PIXBeginNamedEvent(pixColor, "Entity tick %d", levelIndex);
        if (mPlayers->getPlayerCount(level) > 0 || level->hasEntitiesToRemove())
        {
            unsigned long long entityTickStart = System::processTimeInMilliSecs();
            level->tickEntities();
            unsigned long long entityTickEnd = System::processTimeInMilliSecs();
            if (level->mDimension->getType() == DimensionType::OVERWORLD)
                PIXAddNamedCounter(static_cast<float>(entityTickEnd - entityTickStart), "Tick entities");
        }
        PIXEndNamedEvent();

        PIXBeginNamedEvent(pixColor, "Entity tracker tick");
        EntityTracker* tracker = level->getTracker();
        if (tracker != 0)
            tracker->tick(boost::shared_ptr<ServerPlayer>());
        PIXEndNamedEvent();
    }

    Entity::tickExtraWandering();

    PIXBeginNamedEvent(pixColor, "Connection tick");
    if (mConnection != 0)
        mConnection->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(pixColor, "Players tick");
    if (mPlayers != 0)
        mPlayers->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(pixColor, "Game Mode tick");
    if (mMasterGameMode != 0)
        mMasterGameMode->Tick();
    PIXEndNamedEvent();

    instructionSynchronize();
}
void MinecraftServer::chunkPacketManagement_PreTick()
{
    if (mConnection == 0)
        return;

    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >& players =
        mConnection->getPlayers();
    std::stable_sort(players.begin(), players.end(),
        [](const boost::shared_ptr<ServerGamePacketListenerImpl>& first,
           const boost::shared_ptr<ServerGamePacketListenerImpl>& second) {
            INetworkPlayer* firstPlayer = first ? first->getNetworkPlayer() : 0;
            INetworkPlayer* secondPlayer = second ? second->getNetworkPlayer() : 0;
            const int firstAge = firstPlayer == 0 ? 0 : firstPlayer->GetTimeSinceLastChunkPacket_ms();
            const int secondAge = secondPlayer == 0 ? 0 : secondPlayer->GetTimeSinceLastChunkPacket_ms();
            return firstAge > secondAge;
        });
}
void MinecraftServer::run(long long seed, void* argument)
{
    _NetworkGameInitData* initData = static_cast<_NetworkGameInitData*>(argument);
    bool stoppedAfterInit = false;

    unsigned int initFlags = 0;
    bool dedicated = false;
    if (initData != 0)
        gameModeId = initData->m_value6;

    if (initData != 0)
    {
        initFlags = gApp.GetGameHostOption(static_cast<eGameHostOption>(0x20));
        dedicated = initData->m_flag0 != 0;
    }

    if (!initServer(seed, initData, initFlags, dedicated))
    {
        shuttingDown = true;
        stopServer(stoppedAfterInit);
        instructionSynchronize();
        return;
    }

    stoppedAfterInit = true;
    if (mLevels.length != 0 && mLevels.data[0] != 0)
    {
        LevelData* levelData = mLevels.data[0]->getLevelData();
        if (levelData != 0 && !levelData->isInitialized())
        {
            int x = 0;
            int z = 0;
            if (gApp.GetTerrainFeaturePosition(static_cast<_eTerrainFeatureType>(1), &x, &z))
            {
                levelData->setXSpawn(x);
                levelData->setZSpawn(z);
                levelData->setInitialized(true);
            }
        }
    }

    if (GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(0x1d), -1))
        fjSetServerPaused(true);

    unsigned long long lastTime = getCurrentTimeMillis();
    long long accumulatedMilliseconds = 0;

    while (running && !sHaltRequested)
    {
        unsigned long long currentTime = getCurrentTimeMillis();
        long long elapsed = static_cast<long long>(currentTime - lastTime);
        lastTime = currentTime;
        if (elapsed < 0)
            elapsed = 0;
        if (elapsed > 2000)
            elapsed = 2000;
        accumulatedMilliseconds += elapsed;

        if (!serverPaused)
        {
            if (mLevels.length != 0 && mLevels.data[0] != 0
                && mLevels.data[0]->allPlayersAreSleeping())
            {
                tick();
                accumulatedMilliseconds = 0;
            }
            else
            {
                while (accumulatedMilliseconds > 50)
                {
                    accumulatedMilliseconds -= 50;
                    chunkPacketManagement_PreTick();
                    PIXBeginNamedEvent(1.0f, "Server tick");
                    tick();
                    PIXEndNamedEvent();
                }
            }
        }
        else if (mConnection != 0)
        {
            while (accumulatedMilliseconds > 50)
            {
                accumulatedMilliseconds -= 50;
                mConnection->tick();
            }
        }

        for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            const eXuiServerAction action = static_cast<eXuiServerAction>(
                gApp.GetXuiServerAction(playerIndex));
            const int parameter = static_cast<int>(
                reinterpret_cast<uintptr_t>(gApp.GetXuiServerActionParam(playerIndex)));

            if (static_cast<int>(action) == 1)
            {
                if (mPlayers != 0 && mPlayers->getPlayers() != 0
                    && !mPlayers->getPlayers()->empty())
                {
                    (*mPlayers->getPlayers())[0]->drop(
                        not_null_ptr<ItemInstance>(new ItemInstance(Item::byId(parameter), 1, 0)),
                        false);
                }
                gApp.SetXuiServerAction(playerIndex, eXuiServerAction_0, 0);
            }
            else if (static_cast<int>(action) == 2 || static_cast<int>(action) == 3)
            {
                gApp.EnterSaveNotificationSection();
                if (mPlayers != 0)
                    mPlayers->saveAll(
                        reinterpret_cast<ProgressListener*>(Minecraft::GetInstance()->mProgressRenderer),
                        false);
                if (mPlayers != 0)
                    mPlayers->broadcastAll(boost::shared_ptr<Packet>(
                        new UpdateProgressPacket(20)));
                saveAllChunks();
                saveGameRules();
                gApp.LeaveSaveNotificationSection();
                gApp.SetXuiServerAction(playerIndex, eXuiServerAction_0, 0);
            }
        }

        Sleep(1);
    }

    shuttingDown = true;
    stopServer(stoppedAfterInit);
    instructionSynchronize();
}
void MinecraftServer::broadcastStartSavingPacket()
{
    mPlayers->broadcastAll(boost::shared_ptr<Packet>(new ClientboundGameEventPacket(11, 3.0f)));
}

void MinecraftServer::broadcastStopSavingPacket()
{
    mPlayers->broadcastAll(boost::shared_ptr<Packet>(new ClientboundGameEventPacket(12, 3.0f)));
}
unsigned long long MinecraftServer::main(long long seed, void* argument)
{
    MinecraftServer* server = new MinecraftServer();
    gMinecraftServer = server;
    server->run(seed, argument);
    delete server;
    gMinecraftServer = 0;
    ShutdownManager::HasFinished(ShutdownManager::eThreadId_ServerMain);
    return 0;
}
void MinecraftServer::HaltServer(bool force)
{
    sHaltRequested = true;
    sHaltForceFlag = force;
    if (gMinecraftServer != 0)
        gMinecraftServer->halt();
}
void MinecraftServer::info(const std::wstring&) {}
void MinecraftServer::warn(const std::wstring&) {}
std::wstring MinecraftServer::getConsoleName() { return std::wstring(L"CONSOLE"); }

void MinecraftServer::setLevel(int dimension, ServerLevel* level)
{
    unsigned int index = dimension == -1 ? 1 : dimension == 1 ? 2 : 0;
    if (index >= mLevels.length)
        mLevels.resize(index + 1);
    mLevels.data[index] = level;
}

bool MinecraftServer::flagEntitiesToBeRemoved(unsigned int* id)
{
    bool removed = false;
    unsigned int i;
    for (i = 0; i < mLevels.length; ++i)
        if (mLevels.data[i] != 0)
            mLevels.data[i]->flagEntitiesToBeRemoved(id, &removed);
    return removed;
}

void MinecraftServer::addPostProcessRequest(ChunkSource* source, ChunkGenerator* generator, int x, int z)
{
    postProcessRequest request;
    request.x = x;
    request.z = z;
    request.source = source;
    request.generator = generator;
    postProcessRequests.push_back(request);
}

bool MinecraftServer::chunkPacketManagement_CanSendTo(INetworkPlayer* player) { return player != 0 && player->GetOutstandingAckCount() < 2; }
void MinecraftServer::chunkPacketManagement_DidSendTo(INetworkPlayer* player) { if (player != 0) player->SentChunkPacket(); }
void MinecraftServer::Suspend() { suspending = true; saveAllChunks(); suspending = false; }
