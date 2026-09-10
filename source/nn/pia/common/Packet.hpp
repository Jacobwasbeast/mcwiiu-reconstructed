#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_PACKET_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_PACKET_HPP

#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/common/StationAddress.hpp"

namespace nn { namespace pia { namespace common {

class Packet
{
public:
    enum
    {
        cHeaderSize = 36,
        cDataSizeMax = 1472,
        cPayloadSizeMax = 1436,
        cSignature = 0x32ab9864
    };

    class Header
    {
    public:
        enum { cSerializedSize = 36 };

        virtual ~Header();
        virtual Result Serialize(unsigned char* buffer, unsigned int* size) const;
        virtual Result Deserialize(const unsigned char* buffer);
        virtual unsigned int GetSerializedSize() const;

        unsigned int m_signature;
        unsigned char m_version;
        unsigned char m_field09;
        unsigned short m_field0a;
        unsigned short m_field0c;
        unsigned short m_field0e;
        unsigned char m_field10[8];
        unsigned char m_field18[16];
    };

    class PacketData
    {
    public:
        void Copy(const PacketData& packetData);
        unsigned char* GetPayload();
        const unsigned char* GetPayload() const;
        const unsigned char* GetPayload(unsigned int offset) const;
        void SetPayload(const void* payload, unsigned int size);
        void SerializeHeader();
        void DeserializeHeader();

        Header m_header;
        unsigned char m_data[cDataSizeMax];
    };

    Packet();
    ~Packet();
    void Reset();
    unsigned int IsValid() const;
    unsigned char* AssignPayload(unsigned int size);
    void CopyFrom(const Packet& packet);
    Result Encrypt(const Crypto::Setting& setting);
    Result Decrypt(const Crypto::Setting& setting);
    void Trace(unsigned long long value, bool isVerbose) const;
    int GetPacketNumInNetwork() const;

    PacketData m_packetData;
    unsigned int m_size;
    unsigned int m_stationIndex;
    unsigned int m_stationBitmap;
    StationAddress m_field5f4;
    unsigned char m_field600;
    unsigned char m_field601;
    unsigned char m_field602;
    unsigned char m_field603;
    unsigned char m_field604;
    StationAddress m_field608;
    unsigned char m_field614;
};

}}}

#endif
