#include "game/network/TextureAndGeometryChangePacket.hpp"

#include <sstream>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/PacketListener.hpp"

TextureAndGeometryChangePacket::TextureAndGeometryChangePacket()
    : entityId(-1), geometryName(L""), geometryId(0)
{
}

TextureAndGeometryChangePacket::TextureAndGeometryChangePacket(boost::shared_ptr<Entity> entity,
                                                               const std::wstring& geometryName)
    : entityId(entity->getId()), geometryName(geometryName), geometryId(0)
{
    std::wstring identifier = geometryName.substr(7, geometryName.length());
    std::wstring::size_type separator = identifier.find(L'.');
    if (separator != std::wstring::npos)
    {
        identifier = identifier.substr(0, separator);
    }

    std::wstringstream stream;
    stream << identifier;
    stream >> geometryId;
    geometryId |= 0x80000000u;
}

TextureAndGeometryChangePacket::~TextureAndGeometryChangePacket()
{
}

int TextureAndGeometryChangePacket::getPacketId()
{
    return 161;
}

void TextureAndGeometryChangePacket::read(DataInputStream* input)
{
    entityId = input->readInt();
    geometryId = input->readInt();
    geometryName = Packet::readUtf(input, 32767);
}

void TextureAndGeometryChangePacket::write(DataOutputStream* output)
{
    output->writeInt(entityId);
    output->writeInt(geometryId);
    Packet::writeUtf(geometryName, output);
}

void TextureAndGeometryChangePacket::handle(PacketListener* listener)
{
    listener->handleTextureAndGeometryChange(shared_from_this());
}

int TextureAndGeometryChangePacket::getEstimatedSize()
{
    return geometryName.length() + 8;
}

boost::shared_ptr<Packet> TextureAndGeometryChangePacket::create()
{
    return boost::shared_ptr<Packet>(new TextureAndGeometryChangePacket());
}
