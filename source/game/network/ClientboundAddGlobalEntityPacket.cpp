#include "game/network/ClientboundAddGlobalEntityPacket.hpp"

#include <cmath>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/PacketListener.hpp"

ClientboundAddGlobalEntityPacket::ClientboundAddGlobalEntityPacket()
    : mId(-1), mX(0), mY(0), mZ(0), mType(0)
{
}

ClientboundAddGlobalEntityPacket::ClientboundAddGlobalEntityPacket(boost::shared_ptr<Entity> entity)
    : mId(entity->getId()),
      mX((int)std::floor(entity->mX * 32.0)),
      mY((int)std::floor(entity->mY * 32.0)),
      mZ((int)std::floor(entity->mZ * 32.0)),
      mType(entity->GetType() == 0x0b000001ULL ? 1 : 0)
{
}

ClientboundAddGlobalEntityPacket::~ClientboundAddGlobalEntityPacket()
{
}

int ClientboundAddGlobalEntityPacket::getPacketId()
{
    return 0x47;
}

void ClientboundAddGlobalEntityPacket::read(DataInputStream* input)
{
    mId = input->readInt();
    mType = input->readByte();
    mX = input->readInt();
    mY = input->readInt();
    mZ = input->readInt();
}

void ClientboundAddGlobalEntityPacket::write(DataOutputStream* output)
{
    output->writeInt(mId);
    output->writeByte(mType);
    output->writeInt(mX);
    output->writeInt(mY);
    output->writeInt(mZ);
}

void ClientboundAddGlobalEntityPacket::handle(PacketListener* listener)
{
    listener->handleAddGlobalEntity(shared_from_this());
}

int ClientboundAddGlobalEntityPacket::getEstimatedSize()
{
    return 17;
}

int ClientboundAddGlobalEntityPacket::getId()
{
    return mId;
}

int ClientboundAddGlobalEntityPacket::getX()
{
    return mX;
}

int ClientboundAddGlobalEntityPacket::getY()
{
    return mY;
}

int ClientboundAddGlobalEntityPacket::getZ()
{
    return mZ;
}

int ClientboundAddGlobalEntityPacket::getType()
{
    return mType;
}

boost::shared_ptr<Packet> ClientboundAddGlobalEntityPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundAddGlobalEntityPacket());
}
