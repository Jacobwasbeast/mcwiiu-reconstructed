#include "game/server/MinecraftServer.hpp"

#include <ctime>

#include "game/core/GameRules.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/System.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/network/ServerConnection.hpp"
#include "game/world/Level.hpp"

MinecraftServer* gMinecraftServer = 0;

MinecraftServer::MinecraftServer()
    : mMasterGameMode(0), connection(0), settings(0), players(0), commandDispatcher(0), defaultGameType(GameType::SURVIVAL), maxBuildHeight(256), animals(true), npcsEnabled(true), pvpAllowed(true), flightAllowed(false), serverPaused(false), forceGameType(false), suspending(false), running(false)
{
    levels.reserve(3);
    consoleInputs.reserve(4);
    postProcessRequests.reserve(8);
}

MinecraftServer::~MinecraftServer()
{
    levels.clear();
    consoleInputs.clear();
    postProcessRequests.clear();
    players = 0;
    connection = 0;
    settings = 0;
}

bool MinecraftServer::IsServerPaused() const { return serverPaused; }
void MinecraftServer::setFlightAllowed(bool allowed) { flightAllowed = allowed; }
void MinecraftServer::setPlayers(PlayerList* value) { players = value; }
void MinecraftServer::setAnimals(bool value) { animals = value; }
void MinecraftServer::setNpcsEnabled(bool value) { npcsEnabled = value; }
void MinecraftServer::setPvpAllowed(bool value) { pvpAllowed = value; }
void MinecraftServer::setMaxBuildHeight(int value) { maxBuildHeight = value; }
int MinecraftServer::getMaxBuildHeight() { return maxBuildHeight; }
bool MinecraftServer::isHardcore() { return false; }
bool MinecraftServer::getSpawnMonsters() { return true; }

void MinecraftServer::setLevelDifficulty(ServerLevel* level, const Difficulty* difficulty)
{
    if (level != 0 && difficulty != 0)
        level->setDifficulty(difficulty);
}

void MinecraftServer::setDifficulty(const Difficulty* difficulty)
{
    unsigned int i;
    for (i = 0; i < levels.size(); ++i)
        if (levels[i] != 0)
            setLevelDifficulty(levels[i], difficulty);
}

unsigned int MinecraftServer::runPostUpdate(void*) { return 0; }
void MinecraftServer::postProcessTerminate(ProgressRenderer*) { postProcessRequests.clear(); }
ServerLevel* MinecraftServer::getLevel(int dimension) { return dimension >= 0 && dimension < (int)levels.size() ? levels[dimension] : 0; }
void MinecraftServer::saveGameRules() {}
void MinecraftServer::endProgress() {}
bool MinecraftServer::loadLevel(LevelStorageSource*, const std::wstring&, long long, LevelType*, _NetworkGameInitData*) { return true; }
bool MinecraftServer::initServer(long long, _NetworkGameInitData*, unsigned int, bool) { running = true; return true; }

void MinecraftServer::saveAllChunks()
{
    unsigned int i;
    for (i = 0; i < levels.size(); ++i)
        if (levels[i] != 0)
            levels[i]->saveToDisc(0, true);
}

void MinecraftServer::stopServer(bool) { running = false; serverPaused = false; }
PlayerList* MinecraftServer::getPlayers() { return players; }
void MinecraftServer::setDefaultGameMode(const GameType* value) { defaultGameType = value; }
const GameType* MinecraftServer::getDefaultGameType() { return defaultGameType; }
ServerConnection* MinecraftServer::getConnection() { return connection; }
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
    if (level == 0 || players == 0 || player.get() == 0)
        return false;
    if (players->isOp(player))
        return false;
    BlockPos spawn = level->getSharedSpawnPos();
    int dx = Mth::abs(pos.getX() - spawn.getX());
    int dz = Mth::abs(pos.getZ() - spawn.getZ());
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
    if (dimension < 0)
        dimension = 0;
    if ((unsigned int)dimension >= levels.size())
        levels.resize(dimension + 1, (ServerLevel*)0);
    levels[dimension] = level;
}

bool MinecraftServer::flagEntitiesToBeRemoved(unsigned int* id)
{
    bool result = false;
    unsigned int i;
    for (i = 0; i < levels.size(); ++i)
        if (levels[i] != 0)
            result = levels[i]->flagEntitiesToBeRemoved(id) || result;
    return result;
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
