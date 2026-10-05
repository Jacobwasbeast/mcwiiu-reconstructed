#include "platform/wiiu/TLSStorageCafe.hpp"

#include <coreinit/thread.h>
#include <string.h>

static const unsigned int TLS_SLOT_COUNT = 64;
static const OSThreadSpecificID TLS_VALUES_SLOT = OS_THREAD_SPECIFIC_13;

static unsigned int s_tlsAllocated[TLS_SLOT_COUNT];

static void** threadValues()
{
    void** values = static_cast<void**>(OSGetThreadSpecific(TLS_VALUES_SLOT));
    if (values == 0)
    {
        values = new void*[TLS_SLOT_COUNT];
        memset(values, 0, sizeof(void*) * TLS_SLOT_COUNT);
        OSSetThreadSpecific(TLS_VALUES_SLOT, values);
    }
    return values;
}

TLSStorageCafe* TLSStorageCafe::m_instance = 0;

TLSStorageCafe::TLSStorageCafe()
{
    unsigned int index;
    void** values = threadValues();
    for (index = 0; index < TLS_SLOT_COUNT; ++index)
    {
        s_tlsAllocated[index] = 0;
        values[index] = 0;
    }
}

int TLSStorageCafe::Alloc()
{
    unsigned int index;
    for (index = 0; index < TLS_SLOT_COUNT; ++index)
    {
        if (s_tlsAllocated[index] == 0)
        {
            threadValues()[index] = 0;
            s_tlsAllocated[index] = 1;
            return (int)index;
        }
    }
    return -1;
}

TLSStorageCafe* TLSStorageCafe::Instance()
{
    if (m_instance == 0)
    {
        m_instance = new TLSStorageCafe;
    }
    return m_instance;
}

void TLSStorageCafe::Free(unsigned int index)
{
    if (s_tlsAllocated[index] != 0)
    {
        s_tlsAllocated[index] = 0;
    }
}

unsigned int TLSStorageCafe::GetValue(unsigned int index)
{
    if (s_tlsAllocated[index] == 0)
    {
        return 0;
    }
    return (unsigned int)threadValues()[index];
}

unsigned int TLSStorageCafe::SetValue(unsigned int index, void* value)
{
    if (s_tlsAllocated[index] == 0)
    {
        return 0;
    }
    threadValues()[index] = value;
    return 1;
}

unsigned int TlsAlloc()
{
    return static_cast<unsigned int>(TLSStorageCafe::Instance()->Alloc());
}

void* TlsGetValue(unsigned int index)
{
    return reinterpret_cast<void*>(TLSStorageCafe::Instance()->GetValue(index));
}

int TlsSetValue(unsigned int index, void* value)
{
    return static_cast<int>(TLSStorageCafe::Instance()->SetValue(index, value));
}
