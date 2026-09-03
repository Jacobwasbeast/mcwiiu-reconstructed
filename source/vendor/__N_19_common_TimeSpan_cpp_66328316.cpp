#include "vendor/__N_19_common_TimeSpan_cpp_66328316.hpp"

struct OSSystemInfo
{
    unsigned int busSpeed;
};

OSSystemInfo* OSGetSystemInfo();

int64_t __N_19_common_TimeSpan_cpp_66328316::GetFrequency()
{
    return (int64_t)(OSGetSystemInfo()->busSpeed / 4);
}
