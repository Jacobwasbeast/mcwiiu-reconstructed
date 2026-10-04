#include "nn/pia/nat/Api.hpp"
#include "nn/pia/nat/NatProbeData.hpp"

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace nat {

NatProbeData::NatProbeData()
    : m_connectionId(0), m_type(0), m_time()
{
}

common::Result NatProbeData::Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (size == 0)
        return cResultInvalidArgument;
    if (maxSize < cProbeMessageSize)
        return cResultInvalidArgument;

    common::serializeU32(buffer, m_connectionId);
    buffer[4] = m_type;
    for (unsigned int i = 0; i < 3; ++i)
        buffer[5 + i] = m_extra[i];
    common::serializeU64(buffer + 8, m_time.m_tick);
    *size = cProbeMessageSize;
    return 0;
}

common::Result NatProbeData::Deserialize(const unsigned char* buffer, unsigned int size)
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (size < cProbeMessageSize)
        return cResultInvalidArgument;

    m_connectionId = common::deserializeU32(buffer);
    m_type = buffer[4];
    for (unsigned int i = 0; i < 3; ++i)
        m_extra[i] = buffer[5 + i];
    m_time.m_tick = common::deserializeU64(buffer + 8);
    return 0;
}

void NatProbeData::Trace(uint64_t) const
{
}

}}}
