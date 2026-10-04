#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROTOCOL_MESSAGE_ACCESSOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROTOCOL_MESSAGE_ACCESSOR_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/ISerializable.hpp"

#include <string.h>

namespace nn { namespace pia { namespace transport {

class ProtocolMessageAccessor
{
public:
    class Header : public common::ISerializable
    {
    public:
        enum { cSerializedSize = 24 };

        virtual ~Header()
        {
        }

        virtual Result Serialize(unsigned char* buffer, unsigned int* size) const
        {
            if (buffer == 0)
                return static_cast<Result>(0x10c07);
            if (size == 0)
                return static_cast<Result>(0x10c07);

            unsigned char* cursor = buffer;
            *cursor++ = m_flags;
            *cursor++ = (m_messageSize >> 8) & 0xff;
            *cursor++ = m_messageSize & 0xff;
            *cursor++ = (m_destination >> 56) & 0xff;
            *cursor++ = (m_destination >> 48) & 0xff;
            *cursor++ = (m_destination >> 40) & 0xff;
            *cursor++ = (m_destination >> 32) & 0xff;
            *cursor++ = (m_destination >> 24) & 0xff;
            *cursor++ = (m_destination >> 16) & 0xff;
            *cursor++ = (m_destination >> 8) & 0xff;
            *cursor++ = m_destination & 0xff;
            *cursor++ = (m_source >> 56) & 0xff;
            *cursor++ = (m_source >> 48) & 0xff;
            *cursor++ = (m_source >> 40) & 0xff;
            *cursor++ = (m_source >> 32) & 0xff;
            *cursor++ = (m_source >> 24) & 0xff;
            *cursor++ = (m_source >> 16) & 0xff;
            *cursor++ = (m_source >> 8) & 0xff;
            *cursor++ = m_source & 0xff;
            *cursor++ = (m_protocolId >> 8) & 0xff;
            *cursor++ = m_protocolId & 0xff;
            memcpy(cursor, m_field1a, sizeof(m_field1a));
            cursor += sizeof(m_field1a);

            *size = cursor - buffer;
            return 0;
        }

        virtual Result Deserialize(const unsigned char* buffer)
        {
            if (buffer == 0)
                return static_cast<Result>(0x10c07);

            m_flags = buffer[0];
            m_messageSize = (buffer[1] << 8) + buffer[2];
            m_destination =
                static_cast<unsigned long long>(buffer[3]) << 56 |
                static_cast<unsigned long long>(buffer[4]) << 48 |
                static_cast<unsigned long long>(buffer[5]) << 40 |
                static_cast<unsigned long long>(buffer[6]) << 32 |
                static_cast<unsigned long long>(buffer[7]) << 24 |
                static_cast<unsigned long long>(buffer[8]) << 16 |
                static_cast<unsigned long long>(buffer[9]) << 8 |
                buffer[10];
            m_source =
                static_cast<unsigned long long>(buffer[11]) << 56 |
                static_cast<unsigned long long>(buffer[12]) << 48 |
                static_cast<unsigned long long>(buffer[13]) << 40 |
                static_cast<unsigned long long>(buffer[14]) << 32 |
                static_cast<unsigned long long>(buffer[15]) << 24 |
                static_cast<unsigned long long>(buffer[16]) << 16 |
                static_cast<unsigned long long>(buffer[17]) << 8 |
                buffer[18];
            m_protocolId = buffer[19];
            m_protocolId = (m_protocolId << 8) | buffer[20];
            memcpy(m_field1a, buffer + 21, sizeof(m_field1a));

            return 0;
        }

        virtual unsigned int GetSerializedSize() const
        {
            return cSerializedSize;
        }

        unsigned char m_flags;
        unsigned short m_messageSize;
        unsigned long long m_destination;
        unsigned long long m_source;
        unsigned short m_protocolId;
        unsigned char m_field1a[3];
    };
};

}}}

#endif
