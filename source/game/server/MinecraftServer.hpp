#ifndef GAME_SERVER_MINECRAFTSERVER_HPP
#define GAME_SERVER_MINECRAFTSERVER_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/ConsoleSchematicFile.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"

class ChunkGenerator;
class ChunkSource;
class CommandDispatcher;
class C4JEventImpl;
class FillTool;
class Level;
class LevelStorageSource;
class LevelType;
class MasterGameMode;
class Player;
class PlayerList;
class ProgressRenderer;
class ServerConnection;
class ServerLevel;
class Settings;
class ConsoleInputSource;
struct _NetworkGameInitData;

class MinecraftServer
{
public:
    struct postProcessRequest
    {
        int x;
        int z;
        ChunkSource* source;
        ChunkGenerator* generator;
    };

    MinecraftServer();
    virtual ~MinecraftServer();
    bool IsServerPaused() const;
    void setFlightAllowed(bool allowed);
    void setPlayers(PlayerList* players);
    void setAnimals(bool animals);
    void setNpcsEnabled(bool enabled);
    void setPvpAllowed(bool allowed);
    void setMaxBuildHeight(int height);
    int getMaxBuildHeight();
    bool isHardcore();
    bool getSpawnMonsters();
    void setLevelDifficulty(ServerLevel* level, const Difficulty* difficulty);
    void setDifficulty(const Difficulty* difficulty);
    static unsigned int runPostUpdate(void* argument);
    void postProcessTerminate(ProgressRenderer* renderer);
    ServerLevel* getLevel(int dimension);
    void saveGameRules();
    void endProgress();
    bool loadLevel(LevelStorageSource* storage, const std::wstring& name, long long seed, LevelType* type, _NetworkGameInitData* initData);
    bool initServer(long long seed, _NetworkGameInitData* initData, unsigned int flags, bool dedicated);
    void saveAllChunks();
    void stopServer(bool save);
    PlayerList* getPlayers();
    void setDefaultGameMode(const GameType* gameType);
    const GameType* getDefaultGameType();
    ServerConnection* getConnection();
    bool isAnimals();
    bool isNpcsEnabled();
    bool isPvpAllowed();
    bool isFlightAllowed();
    bool isCommandBlockEnabled();
    bool isNetherEnabled();
    CommandDispatcher* getCommandDispatcher();
    int getSpawnProtectionRadius();
    bool isUnderSpawnProtection(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player);
    bool getForceGameType();
    static void getCurrentTimeMillis();
    int getSpawnRadius(ServerLevel* level);
    void halt();
    void fjSetServerPaused(bool paused);
    void tick();
    void chunkPacketManagement_PreTick();
    void run(long long seed, void* argument);
    void broadcastStartSavingPacket();
    void broadcastStopSavingPacket();
    static unsigned long long main(long long seed, void* argument);
    static void HaltServer(bool force);
    virtual void info(const std::wstring& message);
    virtual void warn(const std::wstring& message);
    virtual std::wstring getConsoleName();
    void setLevel(int dimension, ServerLevel* level);
    bool flagEntitiesToBeRemoved(unsigned int* id);
    void addPostProcessRequest(ChunkSource* source, ChunkGenerator* generator, int x, int z);
    static bool chunkPacketManagement_CanSendTo(INetworkPlayer* player);
    static void chunkPacketManagement_DidSendTo(INetworkPlayer* player);
    void Suspend();

    void saveDevData();
    void setProgress(const std::wstring& progress, int percent);
    int getOperatorUserPermissionLevel();
    Level* getCommandSenderWorld();
    class Entity* getCommandSenderEntity();
    void setForceGameType(bool force);
    int getPlayerIdleTimeout();
    void setPlayerIdleTimeout(int timeout);
    void chunkPacketManagement_PostTick();
    void handleConsoleInput(const std::wstring& input, ConsoleInputSource* source);
    void handleConsoleInputs();
    std::wstring getFile(const std::wstring& name);
    bool IsSuspending();

    ServerConnection* mConnection;
    Settings* mSettings;
    arrayWithLength<ServerLevel*> mLevels;
    PlayerList* mPlayers;
    int mLoginComparisonValue;

private:
    ConsoleSchematicFile* schematicFiles[8];
    FillTool* fillTools[8];
    Random random;
    int tickCount;
    bool running;
    bool levelLoaded;
public:
    bool shuttingDown;
private:
    bool saving;
    int networkTick;
    std::wstring progressName;
    int progressNameLength;
    int progressNameCapacity;
    int progressValue;
    CommandDispatcher* commandDispatcher;
    std::vector<ConsoleInputSource*> consoleInputs;
    int consoleInputIndex;
    unsigned short consoleInputCount;
    bool animals;
    bool npcsEnabled;
    bool pvpAllowed;
    bool flightAllowed;
    unsigned short serverFlags;
    std::wstring statusMessage;
    int statusMessageLength;
    int statusMessageCapacity;
    int maxBuildHeight;
    const Difficulty* difficulty;
    LevelType* levelType;
    const GameType* defaultGameType;
    int gameModeId;
    int playerIdleTimeout;
    ProgressRenderer* progressRenderer;
    LevelStorageSource* levelStorage;
    std::vector<postProcessRequest> postProcessRequests;
    int postProcessState;
    OSFastMutex postProcessLock;
    C4JEventImpl* postProcessEvent;
    bool serverPaused;
    bool forceGameType;
    bool suspending;
    bool stopping;
    long long lastServerTime;
    bool postProcessing;
    bool suspended;
    unsigned short shutdownFlags;

public:
    MasterGameMode* mMasterGameMode;
};

extern MinecraftServer* gMinecraftServer;

#endif
