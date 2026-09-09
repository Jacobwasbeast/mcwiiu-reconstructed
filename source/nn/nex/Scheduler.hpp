#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SCHEDULER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SCHEDULER_HPP

#include <cstdint>
#include "nn/nex/RootObject.hpp"

namespace nn { namespace nex {

class Scheduler
{
public:
    typedef bool (*PreventBlockCallback)(RootObject*, std::uint32_t);

    static std::uint64_t SingleThreadDispatch(std::uint32_t timeout);
    static void GlobalSingleThreadDispatch(std::uint32_t timeout);
    static std::uint32_t PreventBlockCall(bool canBlock, std::uint32_t timeout,
        bool initialResult, PreventBlockCallback callback, RootObject* object,
        bool* result);
};

}}

#endif
