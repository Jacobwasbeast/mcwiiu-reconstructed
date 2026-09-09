#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_HPP

#include "nn/pia.h"
#include "nn/pia/String.hpp"

namespace nn { namespace pia { namespace transport {

struct Station::IdentificationToken
{
    char m_token[32];
};

struct Station::PlayerInfo
{
    PlayerInfo();

    common::FixedString<20> m_name;
    unsigned char m_language;
    unsigned long long m_field70;
    unsigned long long m_field78;
};

}}}

#endif
