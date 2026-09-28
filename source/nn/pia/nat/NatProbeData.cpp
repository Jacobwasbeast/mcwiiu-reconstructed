#include "nn/pia/nat/NatProbeData.hpp"

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace nat {

NatProbeData::NatProbeData()
    : mStorage()
{
}

NatProbeData::~NatProbeData()
{
}

common::Result NatProbeData::Serialize(unsigned char* buffer, unsigned int* result, unsigned int size) const
{
    if (buffer == 0 || result == 0 || size < 16)
    {
        if (result != 0)
            *result = 0x10c07;
        return 0x10c07;
    }

    common::serializeU32(buffer, common::deserializeU32(mStorage));
    buffer[4] = mStorage[4];
    buffer[5] = mStorage[5];
    buffer[6] = mStorage[6];
    buffer[7] = mStorage[7];
    common::serializeU32(buffer + 8, common::deserializeU32(mStorage + 8));
    common::serializeU32(buffer + 12, common::deserializeU32(mStorage + 12));
    *result = 16;
    return 0;
}

common::Result NatProbeData::Deserialize(const unsigned char* buffer, unsigned int size)
{
    if (buffer == 0 || size < 16)
    {
        return 0x10c07;
    }

    common::serializeU32(mStorage, common::deserializeU32(buffer));
    mStorage[4] = buffer[4];
    mStorage[5] = buffer[5];
    mStorage[6] = buffer[6];
    mStorage[7] = buffer[7];
    common::serializeU32(mStorage + 8, common::deserializeU32(buffer + 8));
    common::serializeU32(mStorage + 12, common::deserializeU32(buffer + 12));
    return 0;
}

void NatProbeData::Trace(uint64_t) const
{
}

}}}
