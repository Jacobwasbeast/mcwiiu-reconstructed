#include "game/network/ClientIntentionPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientIntentionPacket::ClientIntentionPacket()
    : Packet(), hostName(), protocolVersion(0), port(0), intention(NULL)
{
}

ClientIntentionPacket::ClientIntentionPacket(int protocolVersionValue, std::wstring hostNameValue, int portValue, ConnectionProtocol* intentionValue)
    : Packet(), hostName(hostNameValue), protocolVersion(protocolVersionValue), port(portValue), intention(intentionValue)
{
}

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

ConnectionProtocol* ClientIntentionPacket::getIntention()
{
    return intention;
}

int ClientIntentionPacket::getProtocolVersion()
{
    return protocolVersion;
}

std::wstring ClientIntentionPacket::getHostName()
{
    return hostName;
}

int ClientIntentionPacket::getPort()
{
    return port;
}
