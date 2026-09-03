#include "game/network/ServerboundContainerClosePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ServerboundContainerClosePacket::ServerboundContainerClosePacket()
    : mContainerId(0)
{
}

ServerboundContainerClosePacket::ServerboundContainerClosePacket(int containerId)
    : mContainerId(containerId)
{
}

ServerboundContainerClosePacket::~ServerboundContainerClosePacket()
{
}

boost::shared_ptr<Packet> ServerboundContainerClosePacket::create()
{
    return boost::shared_ptr<Packet>(new ServerboundContainerClosePacket());
}

void ServerboundContainerClosePacket::read(DataInputStream* input)
{
    mContainerId = input->readUnsignedByte();
}

void ServerboundContainerClosePacket::write(DataOutputStream* output)
{
    output->writeByte(static_cast<unsigned char>(mContainerId));
}

void ServerboundContainerClosePacket::handle(PacketListener* listener)
{
    listener->handleContainerClose(shared_from_this());
}

int ServerboundContainerClosePacket::getPacketId()
{
    return 224;
}

int ServerboundContainerClosePacket::getContainerId()
{
    return mContainerId;
}
