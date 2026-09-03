#include "vendor/__N_16_peadTickSpan_cpp_70a0e4dc.hpp"

struct OSSystemInfo
{
    unsigned int busSpeed;
};

OSSystemInfo* OSGetSystemInfo();

int64_t __N_16_peadTickSpan_cpp_70a0e4dc::getFrequency_()
{
    return (int64_t)(OSGetSystemInfo()->busSpeed / 4);
}
