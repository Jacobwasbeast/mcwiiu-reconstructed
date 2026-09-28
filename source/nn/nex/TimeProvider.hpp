#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TIMEPROVIDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TIMEPROVIDER_HPP

#include <stdint.h>

namespace nn
{
namespace nex
{

class TimeProvider
{
public:
    virtual ~TimeProvider();
    virtual uint64_t GetTime() = 0;
};

}
}

#endif
