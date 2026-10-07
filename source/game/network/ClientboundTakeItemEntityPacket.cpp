#include "game/network/ClientboundTakeItemEntityPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundTakeItemEntityPacket::ClientboundTakeItemEntityPacket()
{
    amount = -1;
    itemId = -1;
    playerId = -1;
}

ClientboundTakeItemEntityPacket::ClientboundTakeItemEntityPacket(int itemId, int playerId, int amount)
{
    this->itemId = itemId;
    this->amount = amount;
    this->playerId = playerId;
}

int ClientboundTakeItemEntityPacket::getPacketId()
{
    return 22;
}

void ClientboundTakeItemEntityPacket::read(DataInputStream* input)
{
    itemId = input->readVarInt();
    playerId = input->readVarInt();
    amount = input->readVarInt();
}

void ClientboundTakeItemEntityPacket::write(DataOutputStream* output)
{
    output->writeVarInt(itemId);
    output->writeVarInt(playerId);
    output->writeVarInt(amount);
}

void ClientboundTakeItemEntityPacket::handle(PacketListener* listener)
{
    listener->handleTakeItemEntity(shared_from_this());
}

int ClientboundTakeItemEntityPacket::getEstimatedSize()
{
    return 8;
}

int ClientboundTakeItemEntityPacket::getItemId()
{
    return itemId;
}

int ClientboundTakeItemEntityPacket::getPlayerId()
{
    return playerId;
}

int ClientboundTakeItemEntityPacket::getAmount()
{
    return amount;
}

boost::shared_ptr<Packet> ClientboundTakeItemEntityPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundTakeItemEntityPacket());
}
