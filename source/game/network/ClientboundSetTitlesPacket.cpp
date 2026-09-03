#include "game/network/ClientboundSetTitlesPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetTitlesPacket::ClientboundSetTitlesPacket()
    : type(TITLE), text(), fadeIn(0), stay(0), fadeOut(0)
{
}

ClientboundSetTitlesPacket::ClientboundSetTitlesPacket(Type type, const std::wstring& text, int fadeIn, int stay, int fadeOut)
{
    this->type = type;
    this->text = text;
    this->fadeIn = fadeIn;
    this->stay = stay;
    this->fadeOut = fadeOut;
}

ClientboundSetTitlesPacket::~ClientboundSetTitlesPacket()
{
}

int ClientboundSetTitlesPacket::getPacketId()
{
    return 219;
}

void ClientboundSetTitlesPacket::read(DataInputStream* input)
{
    type = static_cast<Type>(input->readInt());

    if (type < TIMES)
    {
        text = Packet::readUtf(input, 32767);
    }

    if (type == TIMES)
    {
        fadeIn = input->readInt();
        stay = input->readInt();
        fadeOut = input->readInt();
    }
}

void ClientboundSetTitlesPacket::write(DataOutputStream* output)
{
    output->writeInt(static_cast<int>(type));

    if (type < TIMES)
    {
        Packet::writeUtf(text, output);
    }

    if (type == TIMES)
    {
        output->writeInt(fadeIn);
        output->writeInt(stay);
        output->writeInt(fadeOut);
    }
}

void ClientboundSetTitlesPacket::handle(PacketListener* listener)
{
    listener->handleSetTitles(shared_from_this());
}

boost::shared_ptr<Packet> ClientboundSetTitlesPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundSetTitlesPacket());
}
