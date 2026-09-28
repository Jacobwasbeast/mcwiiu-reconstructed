#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_PARAM_VARIANT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_PARAM_VARIANT_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace inet {

class NexSessionSearchCriteria;

class ParamVariant
{
public:
    ParamVariant();

private:
    friend class NexSessionSearchCriteria;

    union
    {
        unsigned long long m_uint64Value;
        long long m_int64Value;
        unsigned int m_uint32Value;
        int m_int32Value;
    };
    unsigned int m_type;
};

}}}

#endif
