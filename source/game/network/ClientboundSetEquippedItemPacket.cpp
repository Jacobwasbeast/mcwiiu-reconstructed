#include "game/network/ClientboundSetEquippedItemPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetEquippedItemPacket::ClientboundSetEquippedItemPacket()
    : m_entity(0), m_item(), m_slot(0)
{
}

ClientboundSetEquippedItemPacket::ClientboundSetEquippedItemPacket(int entity, const EquipmentSlot* slot,
                                                                   not_null_ptr<ItemInstance> item)
    : m_entity(entity), m_item(item), m_slot(slot)
{
    if (!m_item->isEmpty())
        m_item = m_item->copy();
}

ClientboundSetEquippedItemPacket::~ClientboundSetEquippedItemPacket()
{
}

void ClientboundSetEquippedItemPacket::read(DataInputStream* input)
{
    m_entity = input->readVarInt();
    m_slot = EquipmentSlot::SLOTS[input->readShort()];
    m_item = input->readItem();
}

void ClientboundSetEquippedItemPacket::write(DataOutputStream* output)
{
    output->writeVarInt(m_entity);
    output->writeShort(static_cast<short>(m_slot->getEIndex()));
    output->writeItem(m_item);
}

void ClientboundSetEquippedItemPacket::handle(PacketListener* listener)
{
    listener->handleSetEquippedItem(shared_from_this());
}

int ClientboundSetEquippedItemPacket::getEstimatedSize()
{
    return 8;
}

bool ClientboundSetEquippedItemPacket::canBeInvalidated()
{
    return true;
}

bool ClientboundSetEquippedItemPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    boost::shared_ptr<ClientboundSetEquippedItemPacket> other =
        boost::dynamic_pointer_cast<ClientboundSetEquippedItemPacket>(packet);
    return other && other->m_entity == m_entity && other->m_slot == m_slot;
}

not_null_ptr<ItemInstance> ClientboundSetEquippedItemPacket::getItem()
{
    return m_item;
}

int ClientboundSetEquippedItemPacket::getEntity()
{
    return m_entity;
}

const EquipmentSlot* ClientboundSetEquippedItemPacket::getSlot()
{
    return m_slot;
}

int ClientboundSetEquippedItemPacket::getPacketId()
{
    return 5;
}

boost::shared_ptr<Packet> ClientboundSetEquippedItemPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundSetEquippedItemPacket());
}
