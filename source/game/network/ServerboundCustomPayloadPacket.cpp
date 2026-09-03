#include "game/network/ServerboundCustomPayloadPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/biome/arrayWithLength.hpp"

ServerboundCustomPayloadPacket::ServerboundCustomPayloadPacket()
{
}

ServerboundCustomPayloadPacket::ServerboundCustomPayloadPacket(const std::wstring& identifier, arrayWithLength<unsigned char> data)
{
    this->identifier = identifier;
    this->data = data;
}

ServerboundCustomPayloadPacket::~ServerboundCustomPayloadPacket()
{
    delete[] data.data;
}

int ServerboundCustomPayloadPacket::getPacketId()
{
    return 225;
}

void ServerboundCustomPayloadPacket::read(DataInputStream* input)
{
    identifier = Packet::readUtf(input, 20);
    const short length = input->readShort();

    if (length > 0 && length < 32767)
    {
        if (data.data != NULL)
        {
            delete[] data.data;
        }

        data = arrayWithLength<unsigned char>(length, true);
        input->readFully(data);
    }
}

void ServerboundCustomPayloadPacket::write(DataOutputStream* output)
{
    Packet::writeUtf(identifier, output);
    output->writeShort((short)data.length);

    if (data.data != NULL)
    {
        output->write(data);
    }
}

void ServerboundCustomPayloadPacket::handle(PacketListener* listener)
{
    listener->handleCustomPayload(shared_from_this());
}

std::wstring ServerboundCustomPayloadPacket::getIdentifier()
{
    return identifier;
}

arrayWithLength<unsigned char> ServerboundCustomPayloadPacket::getData()
{
    return data;
}

boost::shared_ptr<Packet> ServerboundCustomPayloadPacket::create()
{
    return boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket());
}
