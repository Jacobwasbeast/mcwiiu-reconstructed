#include "game/network/MoveEntityPacketSmall.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include <boost/pointer_cast.hpp>

MoveEntityPacketSmall::MoveEntityPacketSmall() : mEntityId(-1), mXa(0), mYa(0), mZa(0), myRot(0), mxRot(0), mUnused(0), mOnGround(true) {}
MoveEntityPacketSmall::MoveEntityPacketSmall(int entityId) : mEntityId(entityId), mXa(0), mYa(0), mZa(0), myRot(0), mxRot(0), mUnused(0), mOnGround(true) {}
MoveEntityPacketSmall::~MoveEntityPacketSmall() {}
int MoveEntityPacketSmall::getPacketId() { return 162; }
void MoveEntityPacketSmall::read(DataInputStream* input) { mEntityId = input->readInt(); }
void MoveEntityPacketSmall::write(DataOutputStream* output) { output->writeShort((short)mEntityId); }
void MoveEntityPacketSmall::handle(PacketListener* listener) { listener->handleMoveEntitySmall(shared_from_this()); }
int MoveEntityPacketSmall::getEstimatedSize() { return 3; }
bool MoveEntityPacketSmall::canBeInvalidated() { return true; }
bool MoveEntityPacketSmall::isInvalidatedBy(boost::shared_ptr<Packet> packet) { boost::shared_ptr<MoveEntityPacketSmall> other = boost::dynamic_pointer_cast<MoveEntityPacketSmall>(packet); return other && other->mEntityId == mEntityId; }
bool MoveEntityPacketSmall::isOnGround() const { return mOnGround; }
boost::shared_ptr<Packet> MoveEntityPacketSmall::create() { return boost::shared_ptr<Packet>(new MoveEntityPacketSmall()); }

MoveEntityPacketSmall::Pos::Pos() : MoveEntityPacketSmall() {}
MoveEntityPacketSmall::Pos::Pos(int entityId, signed char xa, signed char ya, signed char za, bool onGround) : MoveEntityPacketSmall(entityId) { mXa = xa; mYa = ya; mZa = za; mOnGround = onGround; }
MoveEntityPacketSmall::Pos::~Pos() {}
int MoveEntityPacketSmall::Pos::getPacketId() { return 163; }
void MoveEntityPacketSmall::Pos::read(DataInputStream* input)
{
    int packed = input->readInt();
    mEntityId = (packed >> 20) & 0xfff;
    mXa = (packed >> 14) & 0x3f;
    if (mXa >= 32) mXa -= 64;
    mYa = (packed >> 7) & 0x7f;
    if (mYa >= 64) mYa -= 128;
    mZa = (packed >> 1) & 0x3f;
    if (mZa >= 32) mZa -= 64;
    mOnGround = (packed & 1) != 0;
}
void MoveEntityPacketSmall::Pos::write(DataOutputStream* output) { int packed = ((mEntityId & 0xfff) << 20) | ((mXa & 0x3f) << 14) | ((mYa & 0x7f) << 7) | ((mZa & 0x3f) << 1) | (mOnGround ? 1 : 0); output->writeInt(packed); }
int MoveEntityPacketSmall::Pos::getEstimatedSize() { return 4; }
boost::shared_ptr<Packet> MoveEntityPacketSmall::Pos::create() { return boost::shared_ptr<Packet>(new Pos()); }

MoveEntityPacketSmall::PosRot::PosRot() : MoveEntityPacketSmall() {}
MoveEntityPacketSmall::PosRot::PosRot(int entityId, signed char xa, signed char ya, signed char za,
                                      signed char yRot, signed char xRot, bool onGround)
    : MoveEntityPacketSmall(entityId)
{
    mXa = xa;
    mYa = ya;
    mZa = za;
    myRot = yRot;
    mxRot = xRot;
    mOnGround = onGround;
}
MoveEntityPacketSmall::PosRot::~PosRot() {}
int MoveEntityPacketSmall::PosRot::getPacketId() { return 165; }
void MoveEntityPacketSmall::PosRot::read(DataInputStream* input)
{
    int packed = input->readInt();
    mEntityId = (packed >> 20) & 0xfff;
    mXa = (packed >> 14) & 0x3f;
    if (mXa >= 32) mXa -= 64;
    mYa = (packed >> 7) & 0x7f;
    if (mYa >= 64) mYa -= 128;
    mZa = (packed >> 1) & 0x3f;
    if (mZa >= 32) mZa -= 64;
    mOnGround = (packed & 1) != 0;
    int rotation = input->readUnsignedShort();
    mxRot = (signed char)((rotation >> 8) & 0xff);
    myRot = (signed char)(rotation & 0xff);
}
void MoveEntityPacketSmall::PosRot::write(DataOutputStream* output)
{
    int packed = ((mEntityId & 0xfff) << 20) | ((mXa & 0x3f) << 14) |
                 ((mYa & 0x7f) << 7) | ((mZa & 0x3f) << 1) | (mOnGround ? 1 : 0);
    output->writeInt(packed);
    output->writeShort((short)(((mxRot & 0xff) << 8) | (myRot & 0xff)));
}
int MoveEntityPacketSmall::PosRot::getEstimatedSize() { return 5; }
boost::shared_ptr<Packet> MoveEntityPacketSmall::PosRot::create() { return boost::shared_ptr<Packet>(new PosRot()); }

MoveEntityPacketSmall::Rot::Rot() : MoveEntityPacketSmall() {}
MoveEntityPacketSmall::Rot::Rot(int entityId, signed char yRot, signed char xRot, bool onGround) : MoveEntityPacketSmall(entityId) { myRot = yRot; mOnGround = onGround; mxRot = xRot; }
MoveEntityPacketSmall::Rot::~Rot() {}
int MoveEntityPacketSmall::Rot::getPacketId() { return 164; }
void MoveEntityPacketSmall::Rot::read(DataInputStream* input) { int packed = input->readUnsignedShort(); mEntityId = (packed >> 4) & 0xfff; mOnGround = (packed & 1) != 0; myRot = (signed char)input->readByte(); }
void MoveEntityPacketSmall::Rot::write(DataOutputStream* output) { output->writeShort((short)(((mEntityId & 0xfff) << 4) | (mOnGround ? 1 : 0))); output->writeByte(myRot); }
int MoveEntityPacketSmall::Rot::getEstimatedSize() { return 3; }
boost::shared_ptr<Packet> MoveEntityPacketSmall::Rot::create() { return boost::shared_ptr<Packet>(new Rot()); }
