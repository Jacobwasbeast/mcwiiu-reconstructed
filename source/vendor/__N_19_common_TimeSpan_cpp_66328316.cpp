#include "vendor/__N_19_common_TimeSpan_cpp_66328316.hpp"

#include <coreinit/systeminfo.h>

int64_t __N_19_common_TimeSpan_cpp_66328316::GetFrequency()
{
    return (int64_t)(OSGetSystemInfo()->busClockSpeed / 4);
}
