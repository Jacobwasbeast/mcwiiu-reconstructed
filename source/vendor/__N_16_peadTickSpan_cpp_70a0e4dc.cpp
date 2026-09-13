#include "vendor/__N_16_peadTickSpan_cpp_70a0e4dc.hpp"

#include <coreinit/systeminfo.h>

int64_t __N_16_peadTickSpan_cpp_70a0e4dc::getFrequency_()
{
    return (int64_t)(OSGetSystemInfo()->busClockSpeed / 4);
}
