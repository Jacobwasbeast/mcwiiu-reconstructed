#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TIMEPROVIDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TIMEPROVIDER_HPP

#include <cstdint>

namespace nn
{
namespace nex
{

class TimeProvider
{
public:
    virtual ~TimeProvider();
    virtual std::uint64_t GetTime() = 0;
};

}
}

#endif
