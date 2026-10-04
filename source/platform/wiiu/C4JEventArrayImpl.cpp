#include "platform/wiiu/C4JEventArrayImpl.hpp"

static unsigned long long elapsedMilliseconds(unsigned long long ticks)
{
    return ticks * 1000ULL / (static_cast<unsigned long long>(OSGetSystemInfo()->busClockSpeed) >> 2);
}

C4JEventArrayImpl::C4JEventArrayImpl(int count, C4JEventArray::EMode mode)
    : m_count(count), m_mode(mode)
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

    int remaining = timeout;
    for (int index = 0; index < m_count; ++index)
    {
        int start = OSGetTick();
        int waitResult = 0;
        if (remaining != 0)
        {
            waitResult = OSWaitEventWithTimeout(&m_events[index], remaining);
        }
        int now = OSGetTick();
        if (waitResult == 0)
        {
            return 0x102;
        }
        remaining -= static_cast<int>(elapsedMilliseconds(now - start));
        if (remaining < 0)
        {
            remaining = 0;
        }
    }
    return 0;
}

unsigned int C4JEventArrayImpl::WaitForAny(int timeout)
{
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

    int start = OSGetTick();
    unsigned long long elapsed;
    do
    {
        for (int index = 0; index < m_count; ++index)
        {
            if (OSWaitEventWithTimeout(&m_events[index], 1000) != 0)
            {
                return 0;
            }
        }
        int now = OSGetTick();
        elapsed = elapsedMilliseconds(now - start);
    } while (elapsed < static_cast<unsigned long long>(static_cast<long long>(timeout)));
    return 0x102;
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
    (void)m_count;
}
