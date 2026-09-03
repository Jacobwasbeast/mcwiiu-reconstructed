#include "game/network/ClientboundSetEntityMotionPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetEntityMotionPacket::ClientboundSetEntityMotionPacket()
{
    _init(0, 0.0, 0.0, 0.0);
}

ClientboundSetEntityMotionPacket::ClientboundSetEntityMotionPacket(boost::shared_ptr<Entity> entity)
{
    _init(entity->getId(), entity->mXd, entity->mYd, entity->mZd);
}

ClientboundSetEntityMotionPacket::ClientboundSetEntityMotionPacket(int id, double xa, double ya, double za)
{
    _init(id, xa, ya, za);
}

ClientboundSetEntityMotionPacket::~ClientboundSetEntityMotionPacket()
{
}

void ClientboundSetEntityMotionPacket::_init(int id, double xa, double ya, double za)
{
    this->id = id;

    if (xa < -3.9)
    {
        xa = -3.9;
    }

    if (ya < -3.9)
    {
        ya = -3.9;
    }

    if (za < -3.9)
    {
        za = -3.9;
    }

    if (xa > 3.9)
    {
        xa = 3.9;
    }

    if (ya > 3.9)
    {
        ya = 3.9;
    }

    if (za > 3.9)
    {
        za = 3.9;
    }

    this->xa = (int)(xa * 8000.0);
    this->ya = (int)(ya * 8000.0);
    this->za = (int)(za * 8000.0);

    if (this->xa < -2048 || this->ya < -2048 || this->za < -2048 || this->xa > 2047 || this->ya > 2047 || this->za > 2047)
    {
        compact = false;
    }
    else
    {
        compact = true;
    }
}

int ClientboundSetEntityMotionPacket::getPacketId()
{
    return 28;
}

void ClientboundSetEntityMotionPacket::read(DataInputStream* input)
{
    int header = input->readShort();

    id = header & 0x7FF;

    if ((header & 0x800) != 0)
    {
        xa = (signed char)input->readByte() * 16;
        ya = (signed char)input->readByte() * 16;
        za = (signed char)input->readByte() * 16;
        compact = true;
    }
    else
    {
        xa = input->readShort();
        ya = input->readShort();
        za = input->readShort();
        compact = false;
    }
}

void ClientboundSetEntityMotionPacket::write(DataOutputStream* output)
{
    if (compact)
    {
        output->writeShort(id | 0x800);
        output->writeByte(xa / 16);
        output->writeByte(ya / 16);
        output->writeByte(za / 16);
    }
    else
    {
        output->writeShort(id);
        output->writeShort(xa);
        output->writeShort(ya);
        output->writeShort(za);
    }
}

void ClientboundSetEntityMotionPacket::handle(PacketListener* listener)
{
    listener->handleSetEntityMotion(shared_from_this());
}

int ClientboundSetEntityMotionPacket::getEstimatedSize()
{
    if (compact)
    {
        return 5;
    }

    return 8;
}

bool ClientboundSetEntityMotionPacket::canBeInvalidated()
{
    return true;
}

bool ClientboundSetEntityMotionPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    boost::shared_ptr<ClientboundSetEntityMotionPacket> other = boost::dynamic_pointer_cast<ClientboundSetEntityMotionPacket>(packet);

    return other->id == id;
}

int ClientboundSetEntityMotionPacket::getId()
{
    return id;
}

int ClientboundSetEntityMotionPacket::getXa()
{
    return xa;
}

int ClientboundSetEntityMotionPacket::getYa()
{
    return ya;
}

int ClientboundSetEntityMotionPacket::getZa()
{
    return za;
}

boost::shared_ptr<Packet> ClientboundSetEntityMotionPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundSetEntityMotionPacket());
}
