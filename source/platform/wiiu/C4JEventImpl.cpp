#include "platform/wiiu/C4JEventImpl.hpp"

void OSInitEvent(OSEvent* pEvent, unsigned int initialValue, int mode);
void OSSignalEvent(OSEvent* pEvent);
void OSResetEvent(OSEvent* pEvent);
void OSWaitEvent(OSEvent* pEvent);
int OSWaitEventWithTimeout(OSEvent* pEvent, unsigned long long timeout);
void Sleep(unsigned int milliseconds);

C4JEventImpl::C4JEventImpl(C4JEvent::EMode mode)
    : m_mode(mode), m_waitCount(0)
{
    OSInitEvent(&m_event, 0, mode != C4JEvent::Mode1);
}

C4JEventImpl::~C4JEventImpl()
{
    while (m_waitCount != 0)
    {
        Sleep(1);
    }
}

void C4JEventImpl::Set()
{
    OSSignalEvent(&m_event);
}

void C4JEventImpl::Clear()
{
    OSResetEvent(&m_event);
}

unsigned int C4JEventImpl::WaitForSignal(int timeout)
{
    ++m_waitCount;
    if (timeout == -1)
    {
        OSWaitEvent(&m_event);
        --m_waitCount;
        return 0;
    }

    int result = OSWaitEventWithTimeout(&m_event, static_cast<unsigned long long>(timeout) * 1000000ULL);
    --m_waitCount;
    if (result != 0)
    {
        return 0;
    }
    return 0x102;
}
