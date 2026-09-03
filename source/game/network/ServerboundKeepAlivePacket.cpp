#include "game/network/ServerboundKeepAlivePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ServerboundKeepAlivePacket::ServerboundKeepAlivePacket()
    : id(0)
{
}

ServerboundKeepAlivePacket::ServerboundKeepAlivePacket(int id)
    : id(id)
{
}

ServerboundKeepAlivePacket::~ServerboundKeepAlivePacket()
{
}

boost::shared_ptr<Packet> ServerboundKeepAlivePacket::create()
{
    return boost::shared_ptr<Packet>(new ServerboundKeepAlivePacket());
}

void ServerboundKeepAlivePacket::read(DataInputStream* input)
{
    id = input->readVarInt();
}

void ServerboundKeepAlivePacket::write(DataOutputStream* output)
{
    output->writeVarInt(id);
}

void ServerboundKeepAlivePacket::handle(PacketListener* listener)
{
    listener->handleKeepAlive(shared_from_this());
}

int ServerboundKeepAlivePacket::getPacketId()
{
    return 226;
}
