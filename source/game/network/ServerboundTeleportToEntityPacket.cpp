#include "game/network/ServerboundTeleportToEntityPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/server/ServerLevel.hpp"

ServerboundTeleportToEntityPacket::ServerboundTeleportToEntityPacket()
    : mPlayerUID(0)
{
}

ServerboundTeleportToEntityPacket::~ServerboundTeleportToEntityPacket()
{
}

void ServerboundTeleportToEntityPacket::read(DataInputStream* input)
{
    mPlayerUID = input->readPlayerUID();
}

void ServerboundTeleportToEntityPacket::write(DataOutputStream* output)
{
    output->writePlayerUID(mPlayerUID);
}

void ServerboundTeleportToEntityPacket::handle(PacketListener* listener)
{
    listener->handleTeleportToEntityPacket(shared_from_this());
}

boost::shared_ptr<ServerPlayer> ServerboundTeleportToEntityPacket::getEntity(ServerLevel* level)
{
    return level->getPlayer(mPlayerUID);
}

int ServerboundTeleportToEntityPacket::getPacketId()
{
    return 231;
}

boost::shared_ptr<Packet> ServerboundTeleportToEntityPacket::create()
{
    return boost::shared_ptr<Packet>(new ServerboundTeleportToEntityPacket());
}
