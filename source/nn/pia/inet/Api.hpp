#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_API_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_API_HPP

#include "nn/pia/transport/StationLocation.hpp"

namespace nn { namespace pia { namespace inet {

bool IsBehindNat(const transport::StationLocation& location);
bool IsPublic(const transport::StationLocation& location);
unsigned char IsGlobal(const transport::StationLocation& location);
bool IsEdmMapping(const transport::StationLocation& location);
bool IsEimMapping(const transport::StationLocation& location);

}}}

#endif
