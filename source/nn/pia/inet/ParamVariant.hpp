#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_PARAM_VARIANT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_PARAM_VARIANT_HPP

#include <cstdint>

namespace nn { namespace pia { namespace inet {

class NexSessionSearchCriteria;

class ParamVariant
{
public:
    ParamVariant();

private:
    friend class NexSessionSearchCriteria;

    std::uint32_t m_words[4];
};

}}}

#endif
