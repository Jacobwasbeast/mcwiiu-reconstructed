#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_THREADVARIABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_THREADVARIABLE_HPP

#include <stdint.h>
#include <functional>
#include <map>
#include <utility>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/ThreadVariableRoot.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class ThreadVariable : public ThreadVariableRoot
{
public:
    typedef std::map<uint32_t, T, std::less<uint32_t>,
                     MemAllocator<std::pair<const uint32_t, T> > > ValueMap;

    explicit ThreadVariable(const T& defaultValue)
        : ThreadVariableRoot()
        , m_defaultValue(defaultValue)
        , m_criticalSection(1)
        , m_values()
    {
        m_values[GetThreadID()] = defaultValue;
    }

    virtual ~ThreadVariable()
    {
        ResetValues();
    }

    T& GetRef()
    {
        m_criticalSection.Enter();

        typename ValueMap::iterator value = m_values.find(GetThreadID());
        if (value == m_values.end())
        {
            m_values[GetThreadID()] = m_defaultValue;
            T& created = m_values[GetThreadID()];
            m_criticalSection.Leave();
            return created;
        }

        m_criticalSection.Leave();
        return value->second;
    }

    T& GetRef() const
    {
        m_criticalSection.Enter();

        typename ValueMap::iterator value = m_values.find(GetThreadID());
        if (value == m_values.end())
        {
            m_values[GetThreadID()] = m_defaultValue;
            T& created = m_values[GetThreadID()];
            m_criticalSection.Leave();
            return created;
        }

        m_criticalSection.Leave();
        return value->second;
    }

    void SetValue(const T& value)
    {
        m_criticalSection.Enter();
        m_values[GetThreadID()] = value;
        m_criticalSection.Leave();
    }

    virtual void ResetValues()
    {
        m_criticalSection.Enter();

        while (m_values.size() != 0)
            m_values.erase(m_values.begin());

        m_criticalSection.Leave();
    }

    virtual void ClearValue()
    {
        m_criticalSection.Enter();

        typename ValueMap::iterator value = m_values.find(GetThreadID());
        if (value != m_values.end())
            m_values.erase(value);

        m_criticalSection.Leave();
    }

    virtual void ClearValue(uint32_t threadId)
    {
        m_criticalSection.Enter();

        typename ValueMap::iterator value = m_values.find(threadId);
        if (value != m_values.end())
            m_values.erase(value);

        m_criticalSection.Leave();
    }

private:
    T m_defaultValue;
    mutable ForcedCriticalSection m_criticalSection;
    mutable ValueMap m_values;
};

}
}

#endif
