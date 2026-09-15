#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_IPACKET_OUTPUT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_IPACKET_OUTPUT_HPP

namespace nn { namespace pia { namespace common {

class IPacketOutput
{
public:
    virtual ~IPacketOutput();
    virtual unsigned int GetMulticastDestinationNum() const;
    virtual unsigned int IsBroadcastSupported() const;
};

}}}

#endif
