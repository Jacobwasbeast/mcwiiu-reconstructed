#include "game/server/MinecraftServer.hpp"

#include <ctime>

#include "game/core/GameRules.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/System.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/network/ServerConnection.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerList.hpp"

MinecraftServer* gMinecraftServer = 0;

MinecraftServer::MinecraftServer()
    : mConnection(0), mSettings(0), mPlayers(0), tickCount(0), running(true), levelLoaded(false), postProcessing(false), saving(false), networkTick(0), progressNameLength(0), progressNameCapacity(0), progressValue(0), commandDispatcher(0), consoleInputIndex(0), consoleInputCount(0), animals(true), npcsEnabled(true), pvpAllowed(true), flightAllowed(false), serverFlags(0), statusMessageLength(0), statusMessageCapacity(0), maxBuildHeight(256), difficulty(0), levelType(0), defaultGameType(GameType::SURVIVAL), gameModeId(0), playerIdleTimeout(0), progressRenderer(0), levelStorage(0), postProcessState(0), postProcessEvent(0), serverPaused(false), forceGameType(false), suspending(false), stopping(false), lastServerTime(0), shuttingDown(false), suspended(false), shutdownFlags(0), mMasterGameMode(0)
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

unsigned int MinecraftServer::runPostUpdate(void*) { return 0; }
void MinecraftServer::postProcessTerminate(ProgressRenderer*) { postProcessRequests.clear(); }
ServerLevel* MinecraftServer::getLevel(int dimension)
{
    unsigned int index = dimension == -1 ? 1 : dimension == 1 ? 2 : 0;
    return index < mLevels.length ? mLevels.data[index] : 0;
}
void MinecraftServer::saveGameRules() {}
void MinecraftServer::endProgress() {}
bool MinecraftServer::loadLevel(LevelStorageSource*, const std::wstring&, long long, LevelType*, _NetworkGameInitData*) { return true; }
bool MinecraftServer::initServer(long long, _NetworkGameInitData*, unsigned int, bool) { running = true; return true; }

void MinecraftServer::saveAllChunks()
{
    unsigned int i;
    for (i = 0; i < mLevels.length; ++i)
        if (mLevels.data[i] != 0)
            mLevels.data[i]->saveToDisc(0, true);
}

void MinecraftServer::stopServer(bool) { running = false; serverPaused = false; }
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
void MinecraftServer::getCurrentTimeMillis() { System::processTimeInMilliSecs(); }
int MinecraftServer::getSpawnRadius(ServerLevel* level) { return level == 0 ? 10 : level->getGameRules()->getInt(15); }
void MinecraftServer::halt() { running = false; }
void MinecraftServer::fjSetServerPaused(bool value) { serverPaused = value; }
void MinecraftServer::tick() { if (running) chunkPacketManagement_PreTick(); }
void MinecraftServer::chunkPacketManagement_PreTick() {}
void MinecraftServer::run(long long seed, void* argument) { if (initServer(seed, (_NetworkGameInitData*)argument, 0, false)) running = true; }
void MinecraftServer::broadcastStartSavingPacket() {}
void MinecraftServer::broadcastStopSavingPacket() {}
unsigned long long MinecraftServer::main(long long seed, void* argument) { MinecraftServer server; gMinecraftServer = &server; server.run(seed, argument); return 0; }
void MinecraftServer::HaltServer(bool) { if (gMinecraftServer != 0) gMinecraftServer->halt(); }
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
