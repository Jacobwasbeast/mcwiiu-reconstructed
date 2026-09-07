#include "platform/wiiu/C4JEventArrayImpl.hpp"

C4JEventArrayImpl::C4JEventArrayImpl(int count, C4JEventArray::EMode mode)
    : m_count(count), m_mode(mode), m_events(0)
{
    m_events = new OSEvent[static_cast<unsigned int>(count)];
    for (int index = 0; index < count; ++index)
    {
        OSInitEvent(&m_events[index], 0, static_cast<OSEventMode>(m_mode != C4JEventArray::Mode1));
    }
}

C4JEventArrayImpl::~C4JEventArrayImpl()
{
}

void C4JEventArrayImpl::Set(int index)
{
    OSSignalEvent(&m_events[index]);
}

void C4JEventArrayImpl::Clear(int index)
{
    OSResetEvent(&m_events[index]);
}

uint64_t C4JEventArrayImpl::SetAll()
{
    for (int index = 0; index < m_count; ++index)
    {
        Set(index);
    }
    return 0;
}

uint64_t C4JEventArrayImpl::ClearAll()
{
    for (int index = 0; index < m_count; ++index)
    {
        Clear(index);
    }
    return 0;
}

unsigned int C4JEventArrayImpl::WaitForAll(int timeout)
{
    if (timeout == -1)
    {
        for (int index = 0; index < m_count; ++index)
        {
            OSWaitEvent(&m_events[index]);
        }
        return 0;
    }

    unsigned long long remaining = static_cast<unsigned long long>(timeout) * 1000000ULL;
    for (int index = 0; index < m_count; ++index)
    {
        unsigned long long start = OSGetTick();
        if (timeout != 0 && OSWaitEventWithTimeout(&m_events[index], remaining) == 0)
        {
            return 0x102;
        }
        unsigned long long elapsed = OSGetTick() - start;
        if (elapsed >= remaining)
        {
            remaining = 0;
        }
        else
        {
            remaining -= elapsed;
        }
    }
    return 0;
}

unsigned int C4JEventArrayImpl::WaitForAny(int timeout)
{
    if (m_count < 1)
    {
        return 0x102;
    }

    if (timeout == -1)
    {
        for (;;)
        {
            for (int index = 0; index < m_count; ++index)
            {
                if (OSWaitEventWithTimeout(&m_events[index], 1000) != 0)
                {
                    return 0;
                }
            }
        }
    }

    unsigned long long start = OSGetTick();
    unsigned long long remaining = static_cast<unsigned long long>(timeout) * 1000000ULL;
    for (;;)
    {
        for (int index = 0; index < m_count; ++index)
        {
            if (OSWaitEventWithTimeout(&m_events[index], 1000) != 0)
            {
                return 0;
            }
        }
        unsigned long long elapsed = OSGetTick() - start;
        if (elapsed >= remaining)
        {
            return 0x102;
        }
    }
}

unsigned int C4JEventArrayImpl::WaitForSingle(int index, int timeout)
{
    if (timeout == -1)
    {
        OSWaitEvent(&m_events[index]);
        return 0;
    }
    OSWaitEventWithTimeout(&m_events[index], static_cast<unsigned long long>(timeout) * 1000000ULL);
    return 0x102;
}

void C4JEventArrayImpl::Cancel()
{
}
