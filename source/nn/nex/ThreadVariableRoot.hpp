#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_THREADVARIABLEROOT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_THREADVARIABLEROOT_HPP

#include <cstdint>

namespace nn
{
namespace nex
{

class ThreadVariableRoot
{
public:
    ThreadVariableRoot();
    virtual ~ThreadVariableRoot();

    static std::uint32_t GetThreadID();

    virtual void ResetValues();
    virtual void ClearValue();
    virtual void ClearValue(std::uint32_t value);

protected:
    friend class ThreadVariableList;
    std::uint32_t m_threadVariableState;
    std::uint32_t m_threadVariableValue;
    void* m_variableOwner;
    ThreadVariableRoot* m_next;
    ThreadVariableRoot* m_previous;
};

}
}

#endif
