#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HIGHRESOLUTIONCLOCK_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HIGHRESOLUTIONCLOCK_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class HighResolutionClock : public RootObject
{
public:
    HighResolutionClock();
    virtual ~HighResolutionClock();

    static uint64_t GetTickCount();
    static uint64_t GetFrequency();
    static uint64_t GetTime();
    static uint64_t GetLowResolutionTime();
    static uint64_t GetHighResolutionTime();
    static void DefaultToHighResolution();

    static bool s_defaultToHighResolution;
};

}
}

#endif
