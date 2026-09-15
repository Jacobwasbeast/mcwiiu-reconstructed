#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATCHECKREPLY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATCHECKREPLY_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace nat {

class NatCheckReply
{
public:
    NatCheckReply();

private:
    uint32_t m_values[5];
};

}}}

#endif
