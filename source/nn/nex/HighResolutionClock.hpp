#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HIGHRESOLUTIONCLOCK_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HIGHRESOLUTIONCLOCK_HPP

#include <cstdint>

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

    static std::uint64_t GetTickCount();
    static std::uint64_t GetFrequency();
    static std::uint64_t GetTime();
    static std::uint64_t GetLowResolutionTime();
    static std::uint64_t GetHighResolutionTime();
    static void DefaultToHighResolution();

    static bool s_defaultToHighResolution;
};

}
}

#endif
