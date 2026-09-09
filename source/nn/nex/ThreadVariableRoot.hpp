#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_THREADVARIABLEROOT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_THREADVARIABLEROOT_HPP

#include <cstdint>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class ThreadVariableRoot : public RootObject
{
public:
    ThreadVariableRoot();
    virtual ~ThreadVariableRoot();

    static std::uint32_t GetThreadID();

    virtual void ResetValues();
    virtual void ClearValue();
    virtual void ClearValue(std::uint32_t threadId);

protected:
    friend class ThreadVariableList;

    ThreadVariableRoot* m_next;
    ThreadVariableRoot* m_previous;
};

}
}

#endif
