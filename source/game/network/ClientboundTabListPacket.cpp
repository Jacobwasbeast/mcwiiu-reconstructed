#include "game/network/ClientboundTabListPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundTabListPacket::ClientboundTabListPacket()
    : header(), footer(), headerFadeIn(0), headerFadeOut(7), footerFadeIn(0), footerFadeOut(7)
{
}

ClientboundTabListPacket::ClientboundTabListPacket(const std::wstring& header)
    : header(header), footer(), headerFadeIn(0), headerFadeOut(7), footerFadeIn(0), footerFadeOut(7)
{
}

ClientboundTabListPacket::~ClientboundTabListPacket()
{
}

int ClientboundTabListPacket::getPacketId()
{
    return 220;
}

void ClientboundTabListPacket::read(DataInputStream* input)
{
    header = Packet::readUtf(input, 32767);
    footer = Packet::readUtf(input, 32767);
}

void ClientboundTabListPacket::write(DataOutputStream* output)
{
    Packet::writeUtf(header, output);
    Packet::writeUtf(footer, output);
}

void ClientboundTabListPacket::handle(PacketListener* listener)
{
    listener->handleTabListCustomisation(shared_from_this());
}

boost::shared_ptr<Packet> ClientboundTabListPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundTabListPacket());
}
