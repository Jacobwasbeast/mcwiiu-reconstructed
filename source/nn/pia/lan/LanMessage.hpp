#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMESSAGE_HPP

#include <stdint.h>

namespace nn
{
namespace pia
{
namespace lan
{

class LanMessage
{
public:
    LanMessage();
    ~LanMessage() {}
    virtual void Serialize(unsigned char* buffer, unsigned int* size) const;
    virtual void Deserialize(const unsigned char* buffer);
    virtual void Trace(unsigned long long value);
    virtual unsigned int GetSerializedSize() const;

private:
    unsigned int m_messageType;
    unsigned char m_payload[11];
};
}
}
}

#endif
