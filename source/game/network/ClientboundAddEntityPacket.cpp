#include "game/network/ClientboundAddEntityPacket.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/PacketListener.hpp"

ClientboundAddEntityPacket::ClientboundAddEntityPacket()
{
}

ClientboundAddEntityPacket::ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int data)
{
    _init(entity, type, data, 0, 0, 0,
          (int)std::floor(entity->mXRot * 256.0f / 360.0f),
          (int)std::floor(entity->mYRot * 256.0f / 360.0f));
}

ClientboundAddEntityPacket::ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int yRot, int xRot, int xa, int ya, int za)
{
    _init(entity, type, -1, xa, ya, za, yRot, xRot);
}

ClientboundAddEntityPacket::ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int data, int yRot, int xRot, int xa, int ya, int za)
{
    _init(entity, type, data, xa, ya, za, yRot, xRot);
}

ClientboundAddEntityPacket::ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int data, const BlockPos& pos, int yRot, int xRot, int xa, int ya, int za)
{
    _init(entity, type, data, xa, ya, za, yRot, xRot);
    mX = pos.x * 32;
    mY = pos.y * 32;
    mZ = pos.z * 32;
}

ClientboundAddEntityPacket::~ClientboundAddEntityPacket()
{
}

void ClientboundAddEntityPacket::_init(boost::shared_ptr<Entity> entity, int type, int data, int xa, int ya, int za, int yRot, int xRot)
{
    mId = entity->getId();
    mUUID = entity->getUUID();
    mX = (int)std::floor(entity->mX * 32.0);
    mY = (int)std::floor(entity->mY * 32.0);
    mZ = (int)std::floor(entity->mZ * 32.0);
    mType = type;
    mData = data;
    mYRot = (unsigned char)yRot;
    mXRot = (unsigned char)xRot;
    mXa = (int)((std::max(-4.0, std::min(4.0, entity->mXd)) + 4.0) * 65535.0 / 8.0 - 32768.0);
    mYa = (int)((std::max(-4.0, std::min(4.0, entity->mYd)) + 4.0) * 65535.0 / 8.0 - 32768.0);
    mZa = (int)((std::max(-4.0, std::min(4.0, entity->mZd)) + 4.0) * 65535.0 / 8.0 - 32768.0);
}

void ClientboundAddEntityPacket::read(DataInputStream* input)
{
    mId = input->readInt();
    mUUID = input->readUTF();
    mType = input->readUnsignedByte();
    mX = input->readInt();
    mY = input->readInt();
    mZ = input->readInt();
    mYRot = input->readUnsignedByte();
    mXRot = input->readUnsignedByte();
    mData = input->readInt();
    if (mData >= 0)
    {
        mXa = input->readShort();
        mYa = input->readShort();
        mZa = input->readShort();
    }
    else
    {
        mXa = 0;
        mYa = 0;
        mZa = 0;
    }
}

void ClientboundAddEntityPacket::write(DataOutputStream* output)
{
    output->writeShort((short)mId);
    writeUtf(mUUID, output);
    output->writeByte((unsigned char)mType);
    output->writeShort((short)mX);
    output->writeShort((short)mY);
    output->writeShort((short)mZ);
    output->writeByte(mYRot);
    output->writeByte(mXRot);
    output->writeInt(mData);
    if (mData >= 0)
    {
        output->writeShort((short)mXa);
        output->writeShort((short)mYa);
        output->writeShort((short)mZa);
    }
}

void ClientboundAddEntityPacket::handle(PacketListener* listener)
{
    listener->handleAddEntity(shared_from_this());
}

int ClientboundAddEntityPacket::getEstimatedSize()
{
    return mData + 11 < 0 ? 0 : 6;
}

int ClientboundAddEntityPacket::getPacketId()
{
    return 23;
}

int ClientboundAddEntityPacket::getId()
{
    return mId;
}

std::wstring ClientboundAddEntityPacket::getUUID()
{
    return mUUID;
}

int ClientboundAddEntityPacket::getX()
{
    return mX;
}

int ClientboundAddEntityPacket::getY()
{
    return mY;
}

int ClientboundAddEntityPacket::getZ()
{
    return mZ;
}

double ClientboundAddEntityPacket::getXa()
{
    int value = mXa < -32768 ? -32768 : mXa > 32767 ? 32767 : mXa;
    return ((double)value + 32768.0) / 65535.0 * 8.0 - 4.0;
}

double ClientboundAddEntityPacket::getYa()
{
    int value = mYa < -32768 ? -32768 : mYa > 32767 ? 32767 : mYa;
    return ((double)value + 32768.0) / 65535.0 * 8.0 - 4.0;
}

double ClientboundAddEntityPacket::getZa()
{
    int value = mZa < -32768 ? -32768 : mZa > 32767 ? 32767 : mZa;
    return ((double)value + 32768.0) / 65535.0 * 8.0 - 4.0;
}

unsigned char ClientboundAddEntityPacket::getxRot()
{
    return mXRot;
}

unsigned char ClientboundAddEntityPacket::getyRot()
{
    return mYRot;
}

int ClientboundAddEntityPacket::getType()
{
    return mType;
}

int ClientboundAddEntityPacket::getData()
{
    return mData;
}

void ClientboundAddEntityPacket::setXa(int xa)
{
    mXa = xa;
}

void ClientboundAddEntityPacket::setYa(int ya)
{
    mYa = ya;
}

void ClientboundAddEntityPacket::setZa(int za)
{
    mZa = za;
}

void ClientboundAddEntityPacket::setData(int data)
{
    mData = data;
}

boost::shared_ptr<Packet> ClientboundAddEntityPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket());
}
