#include "game/network/ClientboundContainerSetSlotPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundContainerSetSlotPacket::ClientboundContainerSetSlotPacket()
    : containerId(0)
    , slot(0)
{
    item = ItemInstance::EMPTY;
}

ClientboundContainerSetSlotPacket::ClientboundContainerSetSlotPacket(int containerId, int slot,
                                                                       not_null_ptr<ItemInstance> item)
    : containerId(containerId)
    , slot(slot)
{
    this->item = item->isEmpty() ? ItemInstance::EMPTY : item->copy();
}

int ClientboundContainerSetSlotPacket::getPacketId()
{
    return 103;
}

void ClientboundContainerSetSlotPacket::handle(PacketListener* listener)
{
    listener->handleContainerSetSlot(shared_from_this());
}

void ClientboundContainerSetSlotPacket::read(DataInputStream* input)
{
    containerId = static_cast<signed char>(input->readByte());
    slot = input->readShort();
    item = input->readItem();
}

void ClientboundContainerSetSlotPacket::write(DataOutputStream* output)
{
    output->writeByte(containerId);
    output->writeShort(slot);
    output->writeItem(item);
}

int ClientboundContainerSetSlotPacket::getEstimatedSize()
{
    return 8;
}

int ClientboundContainerSetSlotPacket::getContainerId()
{
    return containerId;
}

int ClientboundContainerSetSlotPacket::getSlot()
{
    return slot;
}

not_null_ptr<ItemInstance> ClientboundContainerSetSlotPacket::getItem()
{
    return item;
}

boost::shared_ptr<Packet> ClientboundContainerSetSlotPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundContainerSetSlotPacket());
}
