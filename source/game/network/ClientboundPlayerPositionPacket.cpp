#include "game/network/ClientboundPlayerPositionPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

const ClientboundPlayerPositionPacket::RelativeArgument ClientboundPlayerPositionPacket::RelativeArgument::X(0);
const ClientboundPlayerPositionPacket::RelativeArgument ClientboundPlayerPositionPacket::RelativeArgument::Y(1);
const ClientboundPlayerPositionPacket::RelativeArgument ClientboundPlayerPositionPacket::RelativeArgument::Z(2);
const ClientboundPlayerPositionPacket::RelativeArgument ClientboundPlayerPositionPacket::RelativeArgument::Y_ROT(3);
const ClientboundPlayerPositionPacket::RelativeArgument ClientboundPlayerPositionPacket::RelativeArgument::X_ROT(4);

ClientboundPlayerPositionPacket::RelativeArgument::RelativeArgument(int bit)
{
    this->bit = bit;
}

ClientboundPlayerPositionPacket::ClientboundPlayerPositionPacket()
{
    x = 0.0;
    y = 0.0;
    z = 0.0;
    yRot = 0.0f;
    xRot = 0.0f;
    hasRotation = false;
    id = 0;
}

ClientboundPlayerPositionPacket::ClientboundPlayerPositionPacket(double x, double y, double z, float yRot, float xRot, const boost::unordered_set<const RelativeArgument*>& relativeArguments, int id)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->yRot = yRot;
    this->xRot = xRot;
    this->hasRotation = true;
    this->id = id;
}

ClientboundPlayerPositionPacket::ClientboundPlayerPositionPacket(double x, double y, double z, const boost::unordered_set<const RelativeArgument*>& relativeArguments, int id)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->yRot = 0.0f;
    this->xRot = 0.0f;
    this->hasRotation = false;
    this->id = id;
}

ClientboundPlayerPositionPacket::~ClientboundPlayerPositionPacket()
{
}

int ClientboundPlayerPositionPacket::getPacketId()
{
    return 214;
}

void ClientboundPlayerPositionPacket::read(DataInputStream* input)
{
    x = input->readDouble();
    y = input->readDouble();
    z = input->readDouble();
    hasRotation = input->readBoolean();
    if (hasRotation)
    {
        yRot = input->readFloat();
        xRot = input->readFloat();
    }

    id = input->readVarInt();
}

void ClientboundPlayerPositionPacket::write(DataOutputStream* output)
{
    output->writeDouble(x);
    output->writeDouble(y);
    output->writeDouble(z);
    output->writeBoolean(hasRotation);
    if (hasRotation)
    {
        output->writeFloat(yRot);
        output->writeFloat(xRot);
    }

    output->writeVarInt(id);
}

void ClientboundPlayerPositionPacket::handle(PacketListener* listener)
{
    listener->handleMovePlayer(shared_from_this());
}

double ClientboundPlayerPositionPacket::getX()
{
    return x;
}

double ClientboundPlayerPositionPacket::getY()
{
    return y;
}

double ClientboundPlayerPositionPacket::getZ()
{
    return z;
}

float ClientboundPlayerPositionPacket::getYRot()
{
    return yRot;
}

float ClientboundPlayerPositionPacket::getXRot()
{
    return xRot;
}

int ClientboundPlayerPositionPacket::getId()
{
    return id;
}

bool ClientboundPlayerPositionPacket::hasRot()
{
    return hasRotation;
}

const boost::unordered_set<const ClientboundPlayerPositionPacket::RelativeArgument*>& ClientboundPlayerPositionPacket::getRelativeArguments()
{
    return relativeArguments;
}

boost::shared_ptr<Packet> ClientboundPlayerPositionPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundPlayerPositionPacket());
}
