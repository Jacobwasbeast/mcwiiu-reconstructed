#ifndef NN_PIA_NAT_NATPROBEDATA_HPP
#define NN_PIA_NAT_NATPROBEDATA_HPP

#include <stdint.h>

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace nat {

class NatProbeData
{
public:
    NatProbeData();
    virtual ~NatProbeData();

    common::Result Serialize(unsigned char* buffer, unsigned int* result, unsigned int size) const;
    common::Result Deserialize(const unsigned char* buffer, unsigned int size);
    void Trace(uint64_t value) const;

private:
    unsigned char mStorage[16];
};

}}}

#endif
