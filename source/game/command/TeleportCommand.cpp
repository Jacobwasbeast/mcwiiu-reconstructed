#include "game/command/TeleportCommand.hpp"

#include "game/command/CommandSender.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/generation/PlayerList.hpp"

extern MinecraftServer* gMinecraftServer;

TeleportCommand::~TeleportCommand()
{
}

EGameCommand TeleportCommand::getId()
{
    return eGameCommand_Teleport;
}

void TeleportCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    unsigned char mode = stream.readByte();
    PlayerUID sourceUid = stream.readPlayerUID();
    PlayerList* playerList = gMinecraftServer != NULL ? gMinecraftServer->getPlayers() : NULL;
    boost::shared_ptr<ServerPlayer> source = playerList != NULL ? playerList->getPlayer(sourceUid) : boost::shared_ptr<ServerPlayer>();

    if (mode == 0)
    {
        PlayerUID targetUid = stream.readPlayerUID();
        boost::shared_ptr<ServerPlayer> target = playerList != NULL ? playerList->getPlayer(targetUid) : boost::shared_ptr<ServerPlayer>();
        if (source && target && source->getLevel()->mDimension->getType()->getId() == target->getLevel()->mDimension->getType()->getId()
            && source->isAlive() && target->isAlive())
        {
            if (source->isSleeping())
            {
                source->stopSleepInBed(true, true, false);
            }

            bool sourcePassenger = source->isPassenger();
            bool targetPassenger = target->isPassenger();
            source->stopRiding();
            source->mConnection->teleport(target->mX, target->mY + (sourcePassenger || targetPassenger ? 1.0 : 0.0), target->mZ,
                                          target->mYRot, target->mXRot, true, true);
            if (source == sender)
            {
                std::wstring stringParams[1];
                stringParams[0] = source->getName();
                target->sendMessage(static_cast<ClientboundChatPacket::EChatPacketMessage>(98), NULL, 0, stringParams, 1);
            }
            else
            {
                std::wstring stringParams[1];
                stringParams[0] = target->getName();
                source->sendMessage(static_cast<ClientboundChatPacket::EChatPacketMessage>(97), NULL, 0, stringParams, 1);
            }
        }
    }
    else if (mode == 1)
    {
        float x = stream.readFloat();
        float y = stream.readFloat();
        float z = stream.readFloat();
        if (source && source->isAlive())
        {
            if (source->isSleeping())
            {
                source->stopSleepInBed(true, true, false);
            }

            bool passenger = source->isPassenger();
            source->stopRiding();
            source->mConnection->teleport(x, y + (passenger ? 1.0 : 0.0), z,
                                          source->mYRot, source->mXRot, true, true);
        }
    }
    byteStream.mBuffer.data = NULL;
    byteStream.mBuffer.length = 0;
    byteStream.mCount = 0;
    byteStream.mMark = 0;
    byteStream.mPosition = 0;
}

boost::shared_ptr<GameCommandPacket> TeleportCommand::preparePacket(PlayerUID source, PlayerUID target)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);
    stream.writeByte(0);
    stream.writePlayerUID(source);
    stream.writePlayerUID(target);
    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_Teleport, byteStream.toByteArray()));
}

boost::shared_ptr<GameCommandPacket> TeleportCommand::preparePacket(PlayerUID target, float x, float y, float z)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);
    stream.writeByte(1);
    stream.writePlayerUID(target);
    stream.writeFloat(x);
    stream.writeFloat(y);
    stream.writeFloat(z);
    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_Teleport, byteStream.toByteArray()));
}
