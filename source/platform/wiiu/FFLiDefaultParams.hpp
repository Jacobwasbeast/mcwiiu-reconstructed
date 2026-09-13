#ifndef SOURCE_PLATFORM_WIIU_FFLIDEFAULTPARAMS_HPP
#define SOURCE_PLATFORM_WIIU_FFLIDEFAULTPARAMS_HPP

#include <stdint.h>

class FFLiDefaultParams
{
public:
    static const unsigned char* GetMaterial(int32_t drawType);
    static const unsigned char* GetLight();
    static const unsigned char* GetRim();
};

#endif
