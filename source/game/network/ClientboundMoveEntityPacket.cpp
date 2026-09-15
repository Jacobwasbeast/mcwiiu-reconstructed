#include "game/network/ClientboundMoveEntityPacket.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include <boost/pointer_cast.hpp>

ClientboundMoveEntityPacket::ClientboundMoveEntityPacket()
    : mEntityId(-1), mXa(0), mYa(0), mZa(0), mYRot(0), mXRot(0), mOnGround(false), mHasRotation(false)
{
}

ClientboundMoveEntityPacket::ClientboundMoveEntityPacket(int entityId)
    : mEntityId(entityId), mXa(0), mYa(0), mZa(0), mYRot(0), mXRot(0), mOnGround(false), mHasRotation(false)
{
}

ClientboundMoveEntityPacket::~ClientboundMoveEntityPacket()
{
}

int ClientboundMoveEntityPacket::getPacketId()
{
    return 30;
}

void ClientboundMoveEntityPacket::read(DataInputStream* input)
{
    mEntityId = input->readInt();
}

void ClientboundMoveEntityPacket::write(DataOutputStream* output)
{
    output->writeShort(static_cast<short>(mEntityId));
}

void ClientboundMoveEntityPacket::handle(PacketListener* listener)
{
    listener->handleMoveEntity(shared_from_this());
}

int ClientboundMoveEntityPacket::getEstimatedSize()
{
    return 2;
}

bool ClientboundMoveEntityPacket::canBeInvalidated()
{
    return true;
}
bool ClientboundMoveEntityPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    boost::shared_ptr<ClientboundMoveEntityPacket> other = boost::dynamic_pointer_cast<ClientboundMoveEntityPacket>(packet);
    return other && other->mEntityId == mEntityId;
}
int ClientboundMoveEntityPacket::getEntityId() { return mEntityId; }
int ClientboundMoveEntityPacket::getXa() { return mXa; }
int ClientboundMoveEntityPacket::getYa() { return mYa; }
int ClientboundMoveEntityPacket::getZa() { return mZa; }
unsigned char ClientboundMoveEntityPacket::getyRot() { return mYRot; }
unsigned char ClientboundMoveEntityPacket::getxRot() { return mXRot; }
bool ClientboundMoveEntityPacket::hasRotation() { return mHasRotation; }
bool ClientboundMoveEntityPacket::isOnGround() { return mOnGround; }

boost::shared_ptr<Packet> ClientboundMoveEntityPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundMoveEntityPacket());
}

ClientboundMoveEntityPacket::Pos::Pos() : ClientboundMoveEntityPacket()
{
}

ClientboundMoveEntityPacket::Pos::Pos(int entityId, int xa, int ya, int za, bool onGround)
    : ClientboundMoveEntityPacket(entityId)
{
    mXa = xa;
    mYa = ya;
    mZa = za;
    mOnGround = onGround;
}

ClientboundMoveEntityPacket::Pos::~Pos()
{
}

int ClientboundMoveEntityPacket::Pos::getPacketId()
{
    return 31;
}
void ClientboundMoveEntityPacket::Pos::read(DataInputStream* input)
{
    ClientboundMoveEntityPacket::read(input);
    int packed = input->readInt();
    mXa = (packed >> 22) & 0x3ff;
    if (mXa >= 512) mXa -= 1024;
    mYa = (packed >> 12) & 0x3ff;
    if (mYa >= 512) mYa -= 1024;
    mZa = (packed >> 2) & 0x3ff;
    if (mZa >= 512) mZa -= 1024;
    mOnGround = (packed & 1) != 0;
}
void ClientboundMoveEntityPacket::Pos::write(DataOutputStream* output)
{
    ClientboundMoveEntityPacket::write(output);
    int packed = ((mXa & 0x3ff) << 22) | ((mYa & 0x3ff) << 12) |
                 ((mZa & 0x3ff) << 2) | (mOnGround ? 1 : 0);
    output->writeInt(packed);
}
int ClientboundMoveEntityPacket::Pos::getEstimatedSize()
{
    return 5;
}

boost::shared_ptr<Packet> ClientboundMoveEntityPacket::Pos::create()
{
    return boost::shared_ptr<Packet>(new Pos());
}

ClientboundMoveEntityPacket::Rot::Rot() : ClientboundMoveEntityPacket()
{
    mHasRotation = true;
}

ClientboundMoveEntityPacket::Rot::Rot(int entityId, unsigned char yRot, unsigned char xRot, bool onGround)
    : ClientboundMoveEntityPacket(entityId)
{
    mYRot = yRot;
    mXRot = xRot;
    mOnGround = onGround;
    mHasRotation = true;
}

ClientboundMoveEntityPacket::Rot::~Rot()
{
}

int ClientboundMoveEntityPacket::Rot::getPacketId()
{
    return 32;
}

void ClientboundMoveEntityPacket::Rot::read(DataInputStream* input)
{
    ClientboundMoveEntityPacket::read(input);
    mYRot = input->readByte();
    mXRot = input->readByte();
    mOnGround = input->readByte();
}

void ClientboundMoveEntityPacket::Rot::write(DataOutputStream* output)
{
    ClientboundMoveEntityPacket::write(output);
    output->writeByte(mYRot);
    output->writeByte(mXRot);
    output->writeByte(mOnGround);
}

int ClientboundMoveEntityPacket::Rot::getEstimatedSize()
{
    return 4;
}

boost::shared_ptr<Packet> ClientboundMoveEntityPacket::Rot::create()
{
    return boost::shared_ptr<Packet>(new Rot());
}

ClientboundMoveEntityPacket::PosRot::PosRot() : ClientboundMoveEntityPacket()
{
    mHasRotation = true;
}
ClientboundMoveEntityPacket::PosRot::PosRot(int entityId, int xa, int ya, int za,
                                             unsigned char yRot, unsigned char xRot, bool onGround)
    : ClientboundMoveEntityPacket(entityId)
{
    mXa = xa;
    mYa = ya;
    mZa = za;
    mYRot = yRot;
    mXRot = xRot;
    mOnGround = onGround;
    mHasRotation = true;
}
ClientboundMoveEntityPacket::PosRot::~PosRot()
{
}

int ClientboundMoveEntityPacket::PosRot::getPacketId()
{
    return 33;
}
void ClientboundMoveEntityPacket::PosRot::read(DataInputStream* input)
{
    ClientboundMoveEntityPacket::read(input);
    int packed = input->readInt();
    mXa = (packed >> 22) & 0x3ff;
    if (mXa >= 512) mXa -= 1024;
    mYa = (packed >> 12) & 0x3ff;
    if (mYa >= 512) mYa -= 1024;
    mZa = (packed >> 2) & 0x3ff;
    if (mZa >= 512) mZa -= 1024;
    mOnGround = (packed & 1) != 0;
    mYRot = input->readByte();
    mXRot = input->readByte();
}
void ClientboundMoveEntityPacket::PosRot::write(DataOutputStream* output)
{
    ClientboundMoveEntityPacket::write(output);
    int packed = ((mXa & 0x3ff) << 22) | ((mYa & 0x3ff) << 12) | ((mZa & 0x3ff) << 2) | (mOnGround ? 1 : 0);
    output->writeInt(packed);
    output->writeByte(mYRot);
    output->writeByte(mXRot);
}
int ClientboundMoveEntityPacket::PosRot::getEstimatedSize()
{
    return 7;
}

boost::shared_ptr<Packet> ClientboundMoveEntityPacket::PosRot::create()
{
    return boost::shared_ptr<Packet>(new PosRot());
}
