#include "game/network/ClientIntentionPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientIntentionPacket::~ClientIntentionPacket()
{
}

void ClientIntentionPacket::read(DataInputStream* input)
{
}

void ClientIntentionPacket::write(DataOutputStream* output)
{
}

void ClientIntentionPacket::handle(PacketListener* listener)
{
}
