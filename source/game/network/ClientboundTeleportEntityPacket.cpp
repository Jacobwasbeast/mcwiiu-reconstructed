#include "game/network/ClientboundTeleportEntityPacket.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/entity/mob/Entity.hpp"
#include <boost/pointer_cast.hpp>
#include <cmath>

ClientboundTeleportEntityPacket::ClientboundTeleportEntityPacket() : mId(-1), mX(0), mY(0), mZ(0), myRot(0), mxRot(0), mOnGround(0) {}
ClientboundTeleportEntityPacket::ClientboundTeleportEntityPacket(boost::shared_ptr<Entity> entity)
    : mId(entity->getId()),
      mX((int)std::floor(entity->mX * 32.0)),
      mY((int)std::floor(entity->mY * 32.0)),
      mZ((int)std::floor(entity->mZ * 32.0)),
      myRot((unsigned char)std::floor(entity->mYRot * 256.0f / 360.0f)),
      mxRot((unsigned char)std::floor(entity->mXRot * 256.0f / 360.0f)),
      mOnGround(entity->mOnGround) {}
ClientboundTeleportEntityPacket::~ClientboundTeleportEntityPacket() {}
int ClientboundTeleportEntityPacket::getPacketId() { return 34; }
void ClientboundTeleportEntityPacket::read(DataInputStream* input)
{
    mId = input->readVarInt();
    mX = input->readShort();
    mY = input->readShort();
    mZ = input->readShort();
    myRot = input->readByte();
    mxRot = input->readByte();
    mOnGround = input->readBoolean();
}
void ClientboundTeleportEntityPacket::write(DataOutputStream* output)
{
    output->writeVarInt(mId);
    output->writeShort((short)mX);
    output->writeShort((short)mY);
    output->writeShort((short)mZ);
    output->writeByte(myRot);
    output->writeByte(mxRot);
    output->writeByte(mOnGround);
}
void ClientboundTeleportEntityPacket::handle(PacketListener* listener) { listener->handleTeleportEntity(shared_from_this()); }
int ClientboundTeleportEntityPacket::getEstimatedSize() { return 10; }
bool ClientboundTeleportEntityPacket::canBeInvalidated() { return true; }
bool ClientboundTeleportEntityPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    boost::shared_ptr<ClientboundTeleportEntityPacket> other = boost::dynamic_pointer_cast<ClientboundTeleportEntityPacket>(packet);
    return other && other->mId == mId;
}
int ClientboundTeleportEntityPacket::getId() { return mId; }
int ClientboundTeleportEntityPacket::getX() { return mX; }
int ClientboundTeleportEntityPacket::getY() { return mY; }
int ClientboundTeleportEntityPacket::getZ() { return mZ; }
unsigned char ClientboundTeleportEntityPacket::getyRot() { return myRot; }
unsigned char ClientboundTeleportEntityPacket::getxRot() { return mxRot; }
unsigned char ClientboundTeleportEntityPacket::isOnGround() { return mOnGround; }
boost::shared_ptr<Packet> ClientboundTeleportEntityPacket::create() { return boost::shared_ptr<Packet>(new ClientboundTeleportEntityPacket()); }
