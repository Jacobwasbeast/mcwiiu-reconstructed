#include "game/command/WeatherCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"

class MinecraftServer
{
public:
    arrayWithLength<Level*> mLevels;
};

extern MinecraftServer* gMinecraftServer;

WeatherCommand::~WeatherCommand()
{
}

EGameCommand WeatherCommand::getId()
{
    return eGameCommand_Weather;
}

int WeatherCommand::getPermissionLevel()
{
    return 2;
}

void WeatherCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);

    unsigned char mode = stream.readByte();
    LevelData* levelData = gMinecraftServer->mLevels.data[0]->getLevelData();
    Random random;
    int duration = random.nextInt(600) * 20 + 6000;

    if (mode == 1)
    {
        levelData->setClearWeatherTime(0);
        levelData->setThunderTime(duration);
        levelData->setRainTime(duration);
        levelData->setThundering(true);
        levelData->setRaining(false);
        logAdminAction(sender, (ClientboundChatPacket::EChatPacketMessage)108, NULL, 0, NULL, 0);
    }
    else if (mode == 2)
    {
        levelData->setClearWeatherTime(0);
        levelData->setThunderTime(duration);
        levelData->setRainTime(duration);
        levelData->setThundering(true);
        levelData->setRaining(true);
        logAdminAction(sender, (ClientboundChatPacket::EChatPacketMessage)107, NULL, 0, NULL, 0);
    }
    else
    {
        levelData->setClearWeatherTime(duration);
        levelData->setThunderTime(0);
        levelData->setRainTime(0);
        levelData->setThundering(false);
        levelData->setRaining(false);
        logAdminAction(sender, (ClientboundChatPacket::EChatPacketMessage)106, NULL, 0, NULL, 0);
    }
}

boost::shared_ptr<GameCommandPacket> WeatherCommand::preparePacket(unsigned char mode)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writeByte(mode);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_Weather, byteStream.toByteArray()));
}
