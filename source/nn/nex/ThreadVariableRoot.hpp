#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_THREADVARIABLEROOT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_THREADVARIABLEROOT_HPP

#include <stdint.h>

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

    static uint32_t GetThreadID();

    virtual void ResetValues();
    virtual void ClearValue();
    virtual void ClearValue(uint32_t threadId);

protected:
    friend class ThreadVariableList;

    ThreadVariableRoot* m_next;
    ThreadVariableRoot* m_previous;
};

}
}

#endif
