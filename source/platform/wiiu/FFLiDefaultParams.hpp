#ifndef SOURCE_PLATFORM_WIIU_FFLIDEFAULTPARAMS_HPP
#define SOURCE_PLATFORM_WIIU_FFLIDEFAULTPARAMS_HPP

#include <stdint.h>

class FFLiDefaultParams
{
public:
    static int32_t GetMaterial(int32_t drawType);
    static uint32_t GetLight();
    static uint32_t GetRim();
};

#endif
