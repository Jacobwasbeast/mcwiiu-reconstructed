#include "game/network/ClientboundExplodePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/util/Vec3.hpp"

ClientboundExplodePacket::ClientboundExplodePacket()
{
    x = 0.6;
    y = 0.6;
    z = 0.6;
    power = 0.0f;
    knockbackOnly = false;
    knockbackX = 0.0f;
    knockbackY = 0.0f;
    knockbackZ = 0.0f;
}

ClientboundExplodePacket::ClientboundExplodePacket(double x, double y, double z, float power, Explosion::ToBlowSet* toBlow, Vec3* knockback, bool knockbackOnly)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->power = power;
    this->knockbackOnly = knockbackOnly;
    if (toBlow != NULL)
    {
        this->toBlow.assign(toBlow->begin(), toBlow->end());
    }
    if (knockback != NULL)
    {
        knockbackX = (float)knockback->x;
        knockbackY = (float)knockback->y;
        knockbackZ = (float)knockback->z;
    }
}

ClientboundExplodePacket::~ClientboundExplodePacket()
{
}

void ClientboundExplodePacket::read(DataInputStream* input)
{
    knockbackOnly = input->readBoolean();
    if (!knockbackOnly)
    {
        x = input->readDouble();
        y = input->readDouble();
        z = input->readDouble();
        power = input->readFloat();
        int count = input->readInt();
        for (int i = 0; i < count; ++i)
        {
            int blockX = (signed char)input->readByte() + (int)x;
            int blockY = (signed char)input->readByte() + (int)y;
            int blockZ = (signed char)input->readByte() + (int)z;
            toBlow.push_back(BlockPos(blockX, blockY, blockZ));
        }
    }
    knockbackX = input->readFloat();
    knockbackY = input->readFloat();
    knockbackZ = input->readFloat();
}

void ClientboundExplodePacket::write(DataOutputStream* output)
{
    output->writeBoolean(knockbackOnly);
    if (!knockbackOnly)
    {
        output->writeDouble((float)x);
        output->writeDouble((float)y);
        output->writeDouble((float)z);
        output->writeFloat(power);
        output->writeInt((int)toBlow.size());
        for (std::vector<BlockPos, std::allocator<BlockPos> >::const_iterator it = toBlow.begin(); it != toBlow.end(); ++it)
        {
            output->writeByte((unsigned char)(it->x - (int)x));
            output->writeByte((unsigned char)(it->y - (int)y));
            output->writeByte((unsigned char)(it->z - (int)z));
        }
    }
    output->writeFloat(knockbackX);
    output->writeFloat(knockbackY);
    output->writeFloat(knockbackZ);
}

void ClientboundExplodePacket::handle(PacketListener* listener)
{
    listener->handleExplosion(shared_from_this());
}

int ClientboundExplodePacket::getEstimatedSize()
{
    return (int)toBlow.size() * 3 + 0x2d;
}

double ClientboundExplodePacket::getX()
{
    return x;
}

double ClientboundExplodePacket::getY()
{
    return y;
}

double ClientboundExplodePacket::getZ()
{
    return z;
}

double ClientboundExplodePacket::getPower()
{
    return power;
}

std::vector<BlockPos, std::allocator<BlockPos> >* ClientboundExplodePacket::getToBlow()
{
    return &toBlow;
}

double ClientboundExplodePacket::getKnockbackX()
{
    return knockbackX;
}

double ClientboundExplodePacket::getKnockbackY()
{
    return knockbackY;
}

double ClientboundExplodePacket::getKnockbackZ()
{
    return knockbackZ;
}

int ClientboundExplodePacket::getPacketId()
{
    return 60;
}

boost::shared_ptr<Packet> ClientboundExplodePacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundExplodePacket());
}
