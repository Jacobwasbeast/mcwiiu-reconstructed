#include "game/network/ServerboundSetCarriedItemPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ServerboundSetCarriedItemPacket::ServerboundSetCarriedItemPacket()
{
}

ServerboundSetCarriedItemPacket::ServerboundSetCarriedItemPacket(int slot)
{
    this->slot = slot;
}

ServerboundSetCarriedItemPacket::~ServerboundSetCarriedItemPacket()
{
}

int ServerboundSetCarriedItemPacket::getPacketId()
{
    return 229;
}

void ServerboundSetCarriedItemPacket::read(DataInputStream* input)
{
    slot = input->readShort();
}

void ServerboundSetCarriedItemPacket::write(DataOutputStream* output)
{
    output->writeShort(slot);
}

void ServerboundSetCarriedItemPacket::handle(PacketListener* listener)
{
    listener->handleSetCarriedItem(shared_from_this());
}

int ServerboundSetCarriedItemPacket::getSlot()
{
    return slot;
}

boost::shared_ptr<Packet> ServerboundSetCarriedItemPacket::create()
{
    return boost::shared_ptr<Packet>(new ServerboundSetCarriedItemPacket());
}
