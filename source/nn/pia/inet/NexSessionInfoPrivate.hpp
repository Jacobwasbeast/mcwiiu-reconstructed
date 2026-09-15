#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXSESSIONINFOPRIVATE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXSESSIONINFOPRIVATE_HPP

#include <cstdint>
#include "nn/pia/inet/NexSessionInfo.hpp"

namespace nn { namespace pia { namespace inet {

class NexSessionInfoPrivate : public NexSessionInfo
{
public:
    NexSessionInfoPrivate();
    virtual ~NexSessionInfoPrivate() override;

};

}}}

#endif
