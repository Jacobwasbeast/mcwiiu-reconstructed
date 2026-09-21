#ifndef SOURCE_NN_PIA_NAT_API_HPP
#define SOURCE_NN_PIA_NAT_API_HPP

namespace nn { namespace pia { namespace nat {

struct DebugSetting
{
    unsigned char options[4];
    unsigned int value;
};

const DebugSetting* GetDebugSetting();

}}}

#endif
