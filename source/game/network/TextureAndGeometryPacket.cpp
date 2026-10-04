#include "game/network/TextureAndGeometryPacket.hpp"

#include <sstream>

#include "game/core/DLCSkinFile.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

TextureAndGeometryPacket::TextureAndGeometryPacket()
{
    shouldDelay = true;
    textureName = L"";
    length = 0;
    additionalBoxCount = 0;
    additionalBoxes = NULL;
    data = NULL;
}

TextureAndGeometryPacket::TextureAndGeometryPacket(const std::wstring& textureName, unsigned char* data,
                                                   unsigned int length)
{
    shouldDelay = true;
    this->textureName = textureName;

    std::wstring identifier(textureName, 7, textureName.length());
    identifier = identifier.substr(0, identifier.find(L'.'));

    std::wstringstream stream;
    stream.setf(std::ios_base::dec, std::ios_base::basefield);
    stream << identifier.c_str();
    stream >> geometryId;
    additionalBoxes = NULL;
    this->data = data;
    geometryId |= 0x80000000u;
    this->length = length;
    additionalBoxCount = 0;
}

TextureAndGeometryPacket::TextureAndGeometryPacket(const std::wstring& textureName, unsigned char* data,
                                                   unsigned int length, DLCSkinFile* skinFile)
{
    shouldDelay = true;
    this->textureName = textureName;

    std::wstring identifier(textureName, 7, textureName.length());
    identifier = identifier.substr(0, identifier.find(L'.'));

    std::wstringstream stream;
    stream.setf(std::ios_base::dec, std::ios_base::basefield);
    stream << identifier.c_str();
    stream >> geometryId;
    this->data = data;
    geometryId |= 0x80000000u;
    this->length = length;

    skinAdjustments = skinFile->getSkinAdjustments();
    additionalBoxCount = skinFile->getAdditionalBoxesCount();
    if (additionalBoxCount != 0)
    {
        additionalBoxes = new SKIN_BOX[additionalBoxCount];
        std::vector<SKIN_BOX*>* boxes = skinFile->getAdditionalBoxes();
        SKIN_BOX* destination = additionalBoxes;
        for (std::vector<SKIN_BOX*>::iterator it = boxes->begin(); it != boxes->end(); ++it)
        {
            *destination = **it;
            ++destination;
        }
    }
    else
    {
        additionalBoxes = NULL;
    }
}

TextureAndGeometryPacket::TextureAndGeometryPacket(const std::wstring& textureName, unsigned char* data,
                                                   unsigned int length, std::vector<SKIN_BOX*>* additionalBoxes,
                                                   _SkinAdjustments skinAdjustments)
{
    shouldDelay = true;
    this->textureName = textureName;

    std::wstring identifier(textureName, 7, textureName.length());
    identifier = identifier.substr(0, identifier.find(L'.'));

    std::wstringstream stream;
    stream.setf(std::ios_base::dec, std::ios_base::basefield);
    stream << identifier.c_str();
    stream >> geometryId;
    this->data = data;
    geometryId |= 0x80000000u;
    this->length = length;
    this->skinAdjustments = skinAdjustments;

    if (additionalBoxes == NULL)
    {
        additionalBoxCount = 0;
        this->additionalBoxes = NULL;
    }
    else
    {
        additionalBoxCount = static_cast<unsigned int>(additionalBoxes->size());
        this->additionalBoxes = new SKIN_BOX[additionalBoxCount];
        SKIN_BOX* destination = this->additionalBoxes;
        for (std::vector<SKIN_BOX*>::iterator it = additionalBoxes->begin(); it != additionalBoxes->end(); ++it)
        {
            *destination = **it;
            ++destination;
        }
    }
}

TextureAndGeometryPacket::~TextureAndGeometryPacket()
{
}

int TextureAndGeometryPacket::getPacketId()
{
    return 160;
}

void TextureAndGeometryPacket::handle(PacketListener* listener)
{
    listener->handleTextureAndGeometry(shared_from_this());
}

void TextureAndGeometryPacket::read(DataInputStream* input)
{
    textureName = Packet::readUtf(input, 32767);
    geometryId = input->readInt();
    length = input->readShort();
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

    skinAdjustments.mXMLVersion = input->readInt();
    skinAdjustments.mAnimFlags = input->readInt();
    skinAdjustments.mGameFlags = input->readInt();
    skinAdjustments.mBodyY = input->readFloat();
    skinAdjustments.mHeadY = input->readFloat();
    skinAdjustments.mArm1Y = input->readFloat();
    skinAdjustments.mLeg1Y = input->readFloat();
    skinAdjustments.mTool1Y = input->readFloat();
    skinAdjustments.mArm0Y = input->readFloat();
    skinAdjustments.mLeg0Y = input->readFloat();
    skinAdjustments.mTool0Y = input->readFloat();

    additionalBoxCount = input->readShort();
    if (additionalBoxCount != 0)
    {
        additionalBoxes = new SKIN_BOX[additionalBoxCount];
    }

    unsigned int index = 0;
    while (index < additionalBoxCount)
    {
        additionalBoxes[index].id = input->readShort();
        additionalBoxes[index].x = input->readFloat();
        additionalBoxes[index].y = input->readFloat();
        additionalBoxes[index].z = input->readFloat();
        additionalBoxes[index].height = input->readFloat();
        additionalBoxes[index].width = input->readFloat();
        additionalBoxes[index].depth = input->readFloat();
        additionalBoxes[index].xTexOffs = input->readFloat();
        additionalBoxes[index].yTexOffs = input->readFloat();
        additionalBoxes[index].armourFlags = input->readInt();
        additionalBoxes[index].mirror = input->readShort();
        ++index;
    }
}

void TextureAndGeometryPacket::write(DataOutputStream* output)
{
    Packet::writeUtf(textureName, output);
    output->writeInt(geometryId);
    output->writeShort(static_cast<short>(length));
    unsigned int index = 0;
    while (index < length)
    {
        output->writeByte(data[index]);
        ++index;
    }

    output->writeInt(skinAdjustments.mXMLVersion);
    output->writeInt(skinAdjustments.mAnimFlags);
    output->writeInt(skinAdjustments.mGameFlags);
    output->writeFloat(skinAdjustments.mBodyY);
    output->writeFloat(skinAdjustments.mHeadY);
    output->writeFloat(skinAdjustments.mArm1Y);
    output->writeFloat(skinAdjustments.mLeg1Y);
    output->writeFloat(skinAdjustments.mTool1Y);
    output->writeFloat(skinAdjustments.mArm0Y);
    output->writeFloat(skinAdjustments.mLeg0Y);
    output->writeFloat(skinAdjustments.mTool0Y);
    output->writeShort(static_cast<short>(additionalBoxCount));

    index = 0;
    while (index < additionalBoxCount)
    {
        output->writeShort(static_cast<short>(additionalBoxes[index].id));
        output->writeFloat(additionalBoxes[index].x);
        output->writeFloat(additionalBoxes[index].y);
        output->writeFloat(additionalBoxes[index].z);
        output->writeFloat(additionalBoxes[index].height);
        output->writeFloat(additionalBoxes[index].width);
        output->writeFloat(additionalBoxes[index].depth);
        output->writeFloat(additionalBoxes[index].xTexOffs);
        output->writeFloat(additionalBoxes[index].yTexOffs);
        output->writeInt(additionalBoxes[index].armourFlags);
        output->writeShort(static_cast<short>(additionalBoxes[index].mirror));
        ++index;
    }
}

int TextureAndGeometryPacket::getEstimatedSize()
{
    return 4228;
}

boost::shared_ptr<Packet> TextureAndGeometryPacket::create()
{
    return boost::shared_ptr<Packet>(new TextureAndGeometryPacket());
}
