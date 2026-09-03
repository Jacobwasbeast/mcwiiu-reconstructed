#include "game/network/ClientboundRespawnPacket.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/LevelType.hpp"

ClientboundRespawnPacket::ClientboundRespawnPacket()
    : seed(0), dimension(-1), dimensionType(0), entityId(0), hardcore(0), gameMode(3), previousGameMode(0), difficultyId(0), worldType(1), keepData(1), debug(0), viewDistance(128), simulationDistance(0), x(0.0), y(0.0), z(0.0), yRot(0.0f), xRot(0.0f), positionAndRotation(0), difficulty(0), playerGameType(0), levelType(0)
{
}

ClientboundRespawnPacket::ClientboundRespawnPacket(char dimension, long long seedValue, const GameType* gameType, const Difficulty* difficultyValue, LevelType* levelTypeValue, bool keep, int mode, int previousMode, int difficultyValueId, int type, bool hard, unsigned char world, int view, int simulation, bool debugValue, int entity)
    : seed(seedValue), dimension(dimension), dimensionType(type), entityId(entity), hardcore(hard), gameMode(mode), previousGameMode((short)previousMode), difficultyId(difficultyValueId), worldType(world), keepData(keep), debug(debugValue), viewDistance(view), simulationDistance(simulation), x(0.0), y(0.0), z(0.0), yRot(0.0f), xRot(0.0f), positionAndRotation(0), difficulty(difficultyValue), playerGameType(gameType), levelType(levelTypeValue)
{
}

ClientboundRespawnPacket::ClientboundRespawnPacket(double xValue, double yValue, double zValue, float yRotation, float xRotation, char dimension, long long seedValue, const GameType* gameType, const Difficulty* difficultyValue, LevelType* levelTypeValue, bool keep, int mode, int previousMode, int difficultyValueId, int type, bool hard, unsigned char world, int view, int simulation, bool debugValue, int entity)
    : seed(seedValue), dimension(dimension), dimensionType(type), entityId(entity), hardcore(hard), gameMode(mode), previousGameMode((short)previousMode), difficultyId(difficultyValueId), worldType(world), keepData(keep), debug(debugValue), viewDistance(view), simulationDistance(simulation), x(xValue), y(yValue), z(zValue), yRot(yRotation), xRot(xRotation), positionAndRotation(1), difficulty(difficultyValue), playerGameType(gameType), levelType(levelTypeValue)
{
}

ClientboundRespawnPacket::~ClientboundRespawnPacket()
{
}

void ClientboundRespawnPacket::handle(PacketListener* listener)
{
    listener->handleRespawn(shared_from_this());
}

void ClientboundRespawnPacket::read(DataInputStream* input)
{
    dimension = (char)input->readByte();
    playerGameType = GameType::byId(input->readByte());
    levelType = LevelType::getLevelType(Packet::readUtf(input, 16));
    if (levelType == 0)
        levelType = LevelType::DEFAULT;
    seed = input->readLong();
    difficulty = Difficulty::byId(input->readByte());
    hardcore = input->readBoolean();
    dimensionType = input->readInt();
    gameMode = input->readInt();
    worldType = input->readByte();
    debug = input->readByte();
    viewDistance = input->readInt();
    simulationDistance = input->readInt();
    keepData = input->readBoolean();
    positionAndRotation = (unsigned char)input->readBoolean();
    if (positionAndRotation)
    {
        x = input->readDouble();
        y = input->readDouble();
        z = input->readDouble();
        yRot = input->readFloat();
        xRot = input->readFloat();
    }
    entityId = input->readInt();
}

void ClientboundRespawnPacket::write(DataOutputStream* output)
{
    output->writeByte((unsigned char)dimension);
    output->writeByte((unsigned char)playerGameType->getId());
    Packet::writeUtf(levelType != 0 ? levelType->getGeneratorName() : std::wstring(), output);
    output->writeLong(seed);
    output->writeByte((unsigned char)difficulty->getId());
    output->writeBoolean(hardcore != 0);
    output->writeInt(dimensionType);
    output->writeInt(gameMode);
    output->writeByte(worldType);
    output->writeByte(debug);
    output->writeInt(viewDistance);
    output->writeInt(simulationDistance);
    output->writeBoolean(keepData != 0);
    output->writeBoolean(positionAndRotation != 0);
    if (positionAndRotation)
    {
        output->writeDouble(x);
        output->writeDouble(y);
        output->writeDouble(z);
        output->writeFloat(yRot);
        output->writeFloat(xRot);
    }
    output->writeInt(entityId);
}

int ClientboundRespawnPacket::getEstimatedSize()
{
    return (levelType != 0 ? (int)levelType->getGeneratorName().size() : 0) + 18;
}

int ClientboundRespawnPacket::getDimension() { return (int)dimension; }
const Difficulty* ClientboundRespawnPacket::getDifficulty() { return difficulty; }
const GameType* ClientboundRespawnPacket::getPlayerGameType() { return playerGameType; }
LevelType* ClientboundRespawnPacket::getLevelType() { return levelType; }
bool ClientboundRespawnPacket::shouldKeepOldEntityData() const { return keepData == 1; }
unsigned char ClientboundRespawnPacket::hasPositionAndRotation() const { return positionAndRotation; }
int ClientboundRespawnPacket::getPacketId() { return 9; }

boost::shared_ptr<Packet> ClientboundRespawnPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundRespawnPacket());
}
