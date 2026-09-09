#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_THREADVARIABLELIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_THREADVARIABLELIST_HPP

#include <cstdint>

namespace nn
{
namespace nex
{

class ThreadVariableRoot;
class CriticalSection;

class ThreadVariableList
{
public:
    ThreadVariableList();
    ~ThreadVariableList();

    void AddVariable(ThreadVariableRoot* variable);
    static ThreadVariableList* GetInstanceRef();
    void ClearValue(std::uint32_t value);
    void RemoveVariable(ThreadVariableRoot* variable);
    void ResetValues();
    void ClearValue();

private:
    ThreadVariableRoot* m_first;
    ThreadVariableRoot* m_last;
    std::uint32_t m_count;
    CriticalSection* m_criticalSection;
    bool m_initialized;
};

}
}

#endif
