#ifndef GAME_NETWORK_CLIENTBOUNDRESPAWNPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDRESPAWNPACKET_HPP

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Difficulty;
class GameType;
class LevelType;
class PacketListener;

class ClientboundRespawnPacket : public Packet, public boost::enable_shared_from_this<ClientboundRespawnPacket>
{
public:
    ClientboundRespawnPacket();
    ClientboundRespawnPacket(char dimension, long long seed, const GameType* gameType, const Difficulty* difficulty, LevelType* levelType, bool keepData, int gameMode, int previousGameMode, int difficultyId, int dimensionType, bool hardcore, unsigned char worldType, int viewDistance, int simulationDistance, bool debug, int entityId);
    ClientboundRespawnPacket(double x, double y, double z, float yRot, float xRot, char dimension, long long seed, const GameType* gameType, const Difficulty* difficulty, LevelType* levelType, bool keepData, int gameMode, int previousGameMode, int difficultyId, int dimensionType, bool hardcore, unsigned char worldType, int viewDistance, int simulationDistance, bool debug, int entityId);
    virtual ~ClientboundRespawnPacket();
    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    int getDimension();
    const Difficulty* getDifficulty();
    const GameType* getPlayerGameType();
    LevelType* getLevelType();
    bool shouldKeepOldEntityData() const;
    unsigned char hasPositionAndRotation() const;
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create();

private:
    long long seed;
    char dimension;
    int dimensionType;
    int entityId;
    unsigned char hardcore;
    int gameMode;
    short previousGameMode;
    int difficultyId;
    unsigned char worldType;
    unsigned char keepData;
    unsigned char debug;
    int viewDistance;
    int simulationDistance;
    double x;
    double y;
    double z;
    float yRot;
    float xRot;
    unsigned char positionAndRotation;
    const Difficulty* difficulty;
    const GameType* playerGameType;
    LevelType* levelType;
};

#endif
