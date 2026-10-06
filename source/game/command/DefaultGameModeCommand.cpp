#include "game/command/DefaultGameModeCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/GameType.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/PlayerInfoPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"

extern MinecraftServer* gMinecraftServer;

static const eGameHostOption eGameHostOption_DefaultGameMode =
    eGameHostOption_GameType;
static const ClientboundChatPacket::EChatPacketMessage eChatPacketMessage_AdminAction =
    ClientboundChatPacket::eChatPacketMessage_DefaultGameModeSuccess;

EGameCommand DefaultGameModeCommand::getId()
{
    return eGameCommand_DefaultGameMode;
}

void DefaultGameModeCommand::doSetGameType(const GameType* gameType)
{
    gMinecraftServer->setDefaultGameMode(gameType);
    CGameNetworkManager::sInstance.SetSessionGameModeId(gameType->getId());
    CGameNetworkManager::sInstance.UpdateAndSetGameSessionData(NULL);

    if (gMinecraftServer->getForceGameType())
    {
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it =
                 gMinecraftServer->getPlayers()->mPlayers.begin();
             it != gMinecraftServer->getPlayers()->mPlayers.end();
             ++it)
        {
            (*it)->setGameMode(const_cast<GameType*>(gameType));
            (*it)->mFallDistance = 0.0f;

            boost::shared_ptr<Packet> info(new PlayerInfoPacket(*it));
            gMinecraftServer->getPlayers()->broadcastAll(info);

            (*it)->sendMessage(ClientboundChatPacket::eChatPacketMessage_GameModeChanged,
                               NULL, 0, NULL, 0);
        }
    }
}

void DefaultGameModeCommand::execute(boost::shared_ptr<CommandSender> sender,
                                     arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);

    unsigned char gameType = stream.readByte();
    doSetGameType(GameType::byId(gameType));
    gApp.SetGameHostOption(eGameHostOption_DefaultGameMode, gameType);

    int intParams[1] = { gameType };
    logAdminAction(sender, eChatPacketMessage_AdminAction,
                   intParams, 1, NULL, 0);
    byteStream.mBuffer.data = NULL;
    byteStream.mBuffer.length = 0;
    byteStream.mCount = 0;
    byteStream.mMark = 0;
    byteStream.mPosition = 0;
}

boost::shared_ptr<GameCommandPacket> DefaultGameModeCommand::preparePacket(unsigned char gameType)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writeByte(gameType);

    return boost::shared_ptr<GameCommandPacket>(
        new GameCommandPacket(eGameCommand_DefaultGameMode, byteStream.toByteArray()));
}
