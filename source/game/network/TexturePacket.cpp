#include "game/network/TexturePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

TexturePacket::TexturePacket()
    : textureName(L""), data(NULL), length(0)
{
    compressed = true;
}

TexturePacket::TexturePacket(const std::wstring& textureName, unsigned char* data, unsigned int length)
    : textureName(textureName), data(data), length(length)
{
    compressed = true;
}

TexturePacket::~TexturePacket()
{
    delete[] data;
    data = NULL;
}

int TexturePacket::getPacketId()
{
    return 154;
}

void TexturePacket::handle(PacketListener* listener)
{
    listener->handleTexture(shared_from_this());
}

void TexturePacket::read(DataInputStream* input)
{
    textureName = Packet::readUtf(input, 32767);
    length = input->readInt();
    if (length != 0)
    {
        data = new unsigned char[length];
        unsigned int index = 0;
        while (index < length)
        {
            data[index] = input->readByte();
            ++index;
        }
    }
}

void TexturePacket::write(DataOutputStream* output)
{
    Packet::writeUtf(textureName, output);
    output->writeInt(length);
    unsigned int index = 0;
    while (index < length)
    {
        output->writeByte(data[index]);
        ++index;
    }
}

int TexturePacket::getEstimatedSize()
{
    return 4096;
}

boost::shared_ptr<Packet> TexturePacket::create()
{
    return boost::shared_ptr<Packet>(new TexturePacket());
}
