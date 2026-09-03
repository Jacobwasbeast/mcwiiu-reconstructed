#include "game/network/ClientboundExplodePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/BlockPos.hpp"

ClientboundExplodePacket::ClientboundExplodePacket()
{
    knockbackOnly = false;
    x = 0.0;
    y = 0.0;
    z = 0.0;
    power = 0.0f;
    knockbackX = 0.0f;
    knockbackY = 0.0f;
    knockbackZ = 0.0f;
}

ClientboundExplodePacket::ClientboundExplodePacket(double x, double y, double z, float power, Explosion::ToBlowSet* toBlow, Vec3* knockback, bool knockbackOnly)
{
    this->knockbackOnly = knockbackOnly;
    this->x = x;
    this->y = y;
    this->z = z;
    this->power = power;

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

int ClientboundExplodePacket::getPacketId()
{
    return 60;
}

void ClientboundExplodePacket::read(DataInputStream* input)
{
    knockbackOnly = input->readBoolean();

    if (!knockbackOnly)
    {
        x = input->readFloat();
        y = input->readFloat();
        z = input->readFloat();
        power = input->readFloat();

        int count = input->readInt();
        int originX = (int)x;
        int originY = (int)y;
        int originZ = (int)z;

        for (int i = 0; i < count; ++i)
        {
            int offsetX = (char)input->readByte();
            int offsetY = (char)input->readByte();
            int offsetZ = (char)input->readByte();
            toBlow.push_back(BlockPos(offsetX + originX, offsetY + originY, offsetZ + originZ));
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
        output->writeFloat((float)x);
        output->writeFloat((float)y);
        output->writeFloat((float)z);
        output->writeFloat(power);
        output->writeInt((int)toBlow.size());

        int originX = (int)x;
        int originY = (int)y;
        int originZ = (int)z;

        for (std::vector<BlockPos, std::allocator<BlockPos> >::iterator it = toBlow.begin(); it != toBlow.end(); ++it)
        {
            output->writeByte((unsigned char)(it->x - originX));
            output->writeByte((unsigned char)(it->y - originY));
            output->writeByte((unsigned char)(it->z - originZ));
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
    return (int)toBlow.size() * 3 + 45;
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

float ClientboundExplodePacket::getPower()
{
    return power;
}

std::vector<BlockPos, std::allocator<BlockPos> >* ClientboundExplodePacket::getToBlow()
{
    return &toBlow;
}

float ClientboundExplodePacket::getKnockbackX()
{
    return knockbackX;
}

float ClientboundExplodePacket::getKnockbackY()
{
    return knockbackY;
}

float ClientboundExplodePacket::getKnockbackZ()
{
    return knockbackZ;
}

boost::shared_ptr<Packet> ClientboundExplodePacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundExplodePacket());
}
