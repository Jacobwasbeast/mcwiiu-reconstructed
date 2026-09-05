#include "game/network/PlayerInfoPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/network/INetworkPlayer.hpp"

PlayerInfoPacket::PlayerInfoPacket()
    : mPlayerId(0), mDimension(-1), mPrivileges(0), mEntityId(-1), mGameType(0)
{
}

PlayerInfoPacket::PlayerInfoPacket(unsigned char playerId, short dimension, unsigned int privileges,
                                   unsigned char gameType)
    : mPlayerId(playerId), mDimension(dimension), mPrivileges(privileges), mEntityId(-1), mGameType(gameType)
{
}

PlayerInfoPacket::PlayerInfoPacket(boost::shared_ptr<ServerPlayer> player)
    : mPlayerId(0), mDimension(0), mPrivileges(0), mEntityId(-1), mGameType(0)
{
    if (player->mConnection != NULL && player->mConnection->getNetworkPlayer() != NULL)
        mPlayerId = player->mConnection->getNetworkPlayer()->GetSmallId();
    mDimension = static_cast<short>(player->mDimension);
    mPrivileges = player->getPlayerGamePrivilege((Player::EPlayerGamePrivileges)0x21);
    mEntityId = player->getId();
    mGameType = static_cast<unsigned char>(player->GetGameType()->getId());
}

PlayerInfoPacket::~PlayerInfoPacket()
{
}

void PlayerInfoPacket::read(DataInputStream* input)
{
    mPlayerId = input->readByte();
    mDimension = input->readShort();
    mPrivileges = static_cast<unsigned int>(input->readInt());
    mEntityId = input->readInt();
    mGameType = input->readByte();
}

void PlayerInfoPacket::write(DataOutputStream* output)
{
    output->writeByte(static_cast<unsigned char>(mPlayerId));
    output->writeShort(mDimension);
    output->writeInt(static_cast<int>(mPrivileges));
    output->writeInt(mEntityId);
    output->writeByte(mGameType);
}

void PlayerInfoPacket::handle(PacketListener* listener)
{
    listener->handlePlayerInfo(shared_from_this());
}

int PlayerInfoPacket::getEstimatedSize()
{
    return 13;
}

boost::shared_ptr<Packet> PlayerInfoPacket::create()
{
    return boost::shared_ptr<Packet>(new PlayerInfoPacket());
}

int PlayerInfoPacket::getPacketId()
{
    return 201;
}
