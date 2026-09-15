#include "nn/pia/common/Packet.hpp"

#include "platform/wiiu/pead.hpp"

#include <coreinit/memory.h>

namespace nn { namespace pia { namespace common {

namespace {

const Result cEncryptInvalidPacket = static_cast<Result>(0x10c08);
const Result cEncryptUnsupportedMode = static_cast<Result>(0xc0b);
const Result cDecryptInvalidPacket = static_cast<Result>(0x2c03);
const Result cDecryptUnsupportedMode = static_cast<Result>(0xc0b);
const unsigned int cEncryptedPayloadLimit = Packet::cPayloadSizeMax + 1;

}

Packet::Header::~Header()
{
}

Result Packet::Header::Serialize(unsigned char* buffer, unsigned int* size) const
{
    if (buffer == 0)
        return static_cast<Result>(0x10c07);
    if (size == 0)
        return static_cast<Result>(0x10c07);

    buffer[0] = static_cast<unsigned char>(m_signature >> 24);
    buffer[1] = static_cast<unsigned char>(m_signature >> 16);
    buffer[2] = static_cast<unsigned char>(m_signature >> 8);
    buffer[3] = static_cast<unsigned char>(m_signature);
    buffer[4] = m_version;
    buffer[5] = m_field09;
    buffer[6] = static_cast<unsigned char>(m_field0a >> 8);
    buffer[7] = static_cast<unsigned char>(m_field0a);
    buffer[8] = static_cast<unsigned char>(m_field0c >> 8);
    buffer[9] = static_cast<unsigned char>(m_field0c);
    buffer[10] = static_cast<unsigned char>(m_field0e >> 8);
    buffer[11] = static_cast<unsigned char>(m_field0e);
    unsigned int index;
    for (index = 0; index < sizeof(m_field10); ++index)
        buffer[12 + index] = m_field10[index];
    for (index = 0; index < sizeof(m_field18); ++index)
        buffer[20 + index] = m_field18[index];

    *size = cSerializedSize;
    return 0;
}

Result Packet::Header::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return static_cast<Result>(0x10c07);

    m_signature = (static_cast<unsigned int>(buffer[0]) << 24) |
                  (static_cast<unsigned int>(buffer[1]) << 16) |
                  (static_cast<unsigned int>(buffer[2]) << 8) | buffer[3];
    m_version = buffer[4];
    m_field09 = buffer[5];
    m_field0a = static_cast<unsigned short>(
        (static_cast<unsigned int>(buffer[6]) << 8) | buffer[7]);
    m_field0c = static_cast<unsigned short>(
        (static_cast<unsigned int>(buffer[8]) << 8) | buffer[9]);
    m_field0e = static_cast<unsigned short>(
        (static_cast<unsigned int>(buffer[10]) << 8) | buffer[11]);
    unsigned int index;
    for (index = 0; index < sizeof(m_field10); ++index)
        m_field10[index] = buffer[12 + index];
    for (index = 0; index < sizeof(m_field18); ++index)
        m_field18[index] = buffer[20 + index];

    return 0;
}

unsigned int Packet::Header::GetSerializedSize() const
{
    return cSerializedSize;
}

void Packet::PacketData::Copy(const PacketData& packetData)
{
    m_header.m_signature = packetData.m_header.m_signature;
    m_header.m_version = packetData.m_header.m_version;
    m_header.m_field09 = packetData.m_header.m_field09;
    m_header.m_field0a = packetData.m_header.m_field0a;
    m_header.m_field0c = packetData.m_header.m_field0c;
    m_header.m_field0e = packetData.m_header.m_field0e;
    unsigned int index;
    for (index = 0; index < sizeof(m_header.m_field10); ++index)
        m_header.m_field10[index] = packetData.m_header.m_field10[index];

    OSBlockMove(m_data, packetData.m_data, cDataSizeMax, FALSE);
}

unsigned char* Packet::PacketData::GetPayload()
{
    return m_data + cHeaderSize;
}

const unsigned char* Packet::PacketData::GetPayload(unsigned int offset) const
{
    return m_data + cHeaderSize + offset;
}

void Packet::PacketData::SetPayload(const void* payload, unsigned int size)
{
    OSBlockMove(GetPayload(), payload, size, FALSE);
}

void Packet::PacketData::SerializeHeader()
{
    unsigned int size;
    m_header.Serialize(m_data, &size);
}

void Packet::PacketData::DeserializeHeader()
{
    m_header.Deserialize(m_data);
}

Packet::Packet()
{
    Reset();
}

Packet::~Packet()
{
}

void Packet::Reset()
{
    m_packetData.m_header.m_signature = cSignature;
    m_packetData.m_header.m_version = 1;
    m_packetData.m_header.m_field09 = 0;
    m_packetData.m_header.m_field0a = 0;
    m_packetData.m_header.m_field0c = 0;
    m_packetData.m_header.m_field0e = 0;
    OSBlockSet(m_packetData.m_header.m_field10, 0, sizeof(m_packetData.m_header.m_field10));
    OSBlockSet(m_packetData.m_header.m_field18, 0, sizeof(m_packetData.m_header.m_field18));

    m_size = cHeaderSize;
    m_stationIndex = 0xfd;
    m_stationBitmap = 0;
    m_field5f4.Clear();
    m_field600 = 0;
    m_field601 = 0;
    m_field602 = 0;
    m_field603 = 0;
    m_field604 = 1;
    m_field608.Clear();
    m_field614 = 0;
}

unsigned int Packet::IsValid() const
{
    if (m_packetData.m_header.m_signature != cSignature)
        return 0;
    if (m_packetData.m_header.m_version != 1 && m_packetData.m_header.m_version != 2)
        return 0;
    if (m_size - cHeaderSize > cPayloadSizeMax)
        return 0;
    return 1;
}

unsigned char* Packet::AssignPayload(unsigned int size)
{
    const unsigned int assignedSize = m_size;
    const unsigned int newSize = assignedSize + size;
    if (newSize > cDataSizeMax)
        return 0;
    m_size = newSize;
    return m_packetData.m_data + assignedSize;
}

void Packet::CopyFrom(const Packet& packet)
{
    m_packetData.Copy(packet.m_packetData);
    m_size = packet.m_size;
    m_stationIndex = packet.m_stationIndex;
    m_stationBitmap = packet.m_stationBitmap;
    m_field5f4 = packet.m_field5f4;
    m_field600 = packet.m_field600;
    m_field601 = packet.m_field601;
    m_field602 = packet.m_field602;
    m_field603 = packet.m_field603;
    m_field604 = packet.m_field604;
    m_field608 = packet.m_field608;
    m_field614 = packet.m_field614;
}

Result Packet::Encrypt(const Crypto::Setting& setting)
{
    if (IsValid() == 0 || m_packetData.m_header.m_version != 1)
        return cEncryptInvalidPacket;

    if (setting.m_mode == 0)
        return 0;
    if (setting.m_mode != 1)
        return cEncryptUnsupportedMode;

    const unsigned int payloadSize = m_size - cHeaderSize;
    if (payloadSize < 1)
        return 0;

    const unsigned int blockSize = Crypto::GetBlockSize(setting.m_mode);
    const unsigned int encryptedSize =
        ((payloadSize + blockSize - 1) / blockSize) * blockSize;
    if (encryptedSize >= cEncryptedPayloadLimit)
        return cEncryptInvalidPacket;

    unsigned char source[cPayloadSizeMax];
    OSBlockMove(source, m_packetData.GetPayload(), payloadSize, FALSE);
    const unsigned int paddingSize = encryptedSize - payloadSize;
    if (paddingSize != 0)
        OSBlockSet(source + payloadSize, 0xff, paddingSize);

    const Result result = Crypto::Encrypt(
        m_packetData.GetPayload(), source, encryptedSize,
        m_packetData.m_header.m_field18, sizeof(m_packetData.m_header.m_field18), setting);
    if (result != 0)
        return result;

    m_packetData.m_header.m_version = 2;
    m_size += paddingSize;
    return 0;
}

Result Packet::Decrypt(const Crypto::Setting& setting)
{
    if (IsValid() == 0)
        return cDecryptInvalidPacket;
    if (m_packetData.m_header.m_version == 1)
        return 0;
    if (setting.m_mode != 1)
        return cDecryptUnsupportedMode;
    if (m_packetData.m_header.m_version != 2)
        return cDecryptInvalidPacket;

    const unsigned int encryptedSize = m_size - cHeaderSize;
    if (encryptedSize == 0)
    {
        m_packetData.m_header.m_version = 1;
        return 0;
    }

    const unsigned int blockSize = Crypto::GetBlockSize(setting.m_mode);
    if (encryptedSize % blockSize != 0 || encryptedSize >= cEncryptedPayloadLimit)
        return cDecryptInvalidPacket;

    unsigned char destination[cPayloadSizeMax];
    const Result result = Crypto::Decrypt(
        destination, m_packetData.GetPayload(), encryptedSize,
        m_packetData.m_header.m_field18, sizeof(m_packetData.m_header.m_field18), setting);
    if (result != 0)
        return result;

    m_packetData.SetPayload(destination, encryptedSize);
    m_packetData.m_header.m_version = 1;
    return 0;
}

void Packet::Trace(unsigned long long value, bool isVerbose) const
{
}

int Packet::GetPacketNumInNetwork() const
{
    const int packetNum =
        static_cast<int>(pead::BitFlagUtil::countOnBit(m_stationBitmap));
    if (packetNum < 1)
        return 1;
    return packetNum;
}

}}}
