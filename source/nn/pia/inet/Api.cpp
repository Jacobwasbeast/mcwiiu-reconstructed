#include "nn/pia/inet/Api.hpp"

namespace nn { namespace pia { namespace inet {

bool IsBehindNat(const transport::StationLocation& location)
{
    return (location.m_field25 & 1) != 0;
}

bool IsPublic(const transport::StationLocation& location)
{
    return ((location.m_field25 >> 1) & 1) != 0;
}

unsigned char IsGlobal(const transport::StationLocation& location)
{
    unsigned int global = 0;
    if (IsPublic(location) && !IsBehindNat(location))
        global = 1;
    return global;
}

bool IsEdmMapping(const transport::StationLocation& location)
{
    return location.m_field23 == 2;
}

bool IsEimMapping(const transport::StationLocation& location)
{
    return location.m_field23 == 1;
}

}}}
