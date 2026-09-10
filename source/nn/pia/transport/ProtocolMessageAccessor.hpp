#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROTOCOL_MESSAGE_ACCESSOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROTOCOL_MESSAGE_ACCESSOR_HPP

#include "nn/pia/Reconstruction.hpp"

namespace nn { namespace pia { namespace transport {

class ProtocolMessageAccessor
{
public:
    class Header
    {
    public:
        enum { cSerializedSize = 24 };

        Header();
        virtual ~Header();
        virtual Result Serialize(unsigned char* buffer, unsigned int* size) const;
        virtual Result Deserialize(const unsigned char* buffer);
        virtual unsigned int GetSerializedSize() const;

        unsigned char m_flags;
        unsigned short m_messageSize;
        unsigned long long m_destination;
        unsigned long long m_source;
        ProtocolId m_protocolId;
        unsigned char m_field1a[3];
    };
};

}}}

#endif
