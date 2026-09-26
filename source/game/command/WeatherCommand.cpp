#include "game/command/WeatherCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"

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

    ServerLevel* level = gMinecraftServer->mLevels.data[0];
    LevelData* levelData = level->getLevelData();

    Random random;
    int duration = random.nextInt(600) * 20 + 6000;

    if (mode == 1)
    {
        levelData->setClearWeatherTime(0);
        levelData->setRainTime(duration);
        levelData->setThunderTime(duration);
        levelData->setRaining(true);
        levelData->setThundering(false);

        logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_WeatherRain, NULL, 0, NULL, 0);
    }
    else if (mode == 2)
    {
        levelData->setClearWeatherTime(0);
        levelData->setRainTime(duration);
        levelData->setThunderTime(duration);
        levelData->setRaining(true);
        levelData->setThundering(true);

        logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_WeatherThunder, NULL, 0, NULL, 0);
    }
    else
    {
        levelData->setClearWeatherTime(duration);
        levelData->setRainTime(0);
        levelData->setThunderTime(0);
        levelData->setRaining(false);
        levelData->setThundering(false);

        logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_WeatherClear, NULL, 0, NULL, 0);
    }

    level->tickWeather(true);
    byteStream.mBuffer.data = NULL;
    byteStream.mBuffer.length = 0;
    byteStream.mCount = 0;
    byteStream.mMark = 0;
    byteStream.mPosition = 0;
}

boost::shared_ptr<GameCommandPacket> WeatherCommand::preparePacket(unsigned char mode)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writeByte(mode);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_Weather, byteStream.toByteArray()));
}
