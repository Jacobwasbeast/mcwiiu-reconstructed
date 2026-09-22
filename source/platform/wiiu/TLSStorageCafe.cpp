#include "platform/wiiu/TLSStorageCafe.hpp"

#include "platform/wiiu/CafeThreadLocal.hpp"

#include <coreinit/thread.h>
#include <map>

static unsigned int s_tlsAllocated[64];
namespace
{

std::map<OSThread*, void*>& threadValues(unsigned int index)
{
    static std::map<OSThread*, void*> values[64];
    return values[index];
}

}

TLSStorageCafe* TLSStorageCafe::m_instance = 0;

TLSStorageCafe::TLSStorageCafe()
{
    unsigned int index;
    for (index = 0; index < 64; ++index)
    {
        s_tlsAllocated[index] = 0;
    }
}

int TLSStorageCafe::Alloc()
{
    unsigned int index;
    for (index = 0; index < 64; ++index)
    {
        if (s_tlsAllocated[index] == 0)
        {
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
        m_instance = new TLSStorageCafe();
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
    return (unsigned int)CafeThreadLocal::slot(threadValues(index));
}

unsigned int TLSStorageCafe::SetValue(unsigned int index, void* value)
{
    if (s_tlsAllocated[index] == 0)
    {
        return 0;
    }
    CafeThreadLocal::slot(threadValues(index)) = value;
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
