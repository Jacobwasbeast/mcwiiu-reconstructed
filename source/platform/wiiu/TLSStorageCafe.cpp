#include "platform/wiiu/TLSStorageCafe.hpp"

static unsigned int s_tlsAllocated[64];

__thread void* TLSStorageCafe::m_values[64];
TLSStorageCafe* TLSStorageCafe::m_instance = 0;

TLSStorageCafe::TLSStorageCafe()
{
    unsigned int index;
    for (index = 0; index < 64; ++index)
    {
        s_tlsAllocated[index] = 0;
        m_values[index] = 0;
    }
}

int TLSStorageCafe::Alloc()
{
    unsigned int index;
    for (index = 0; index < 64; ++index)
    {
        if (s_tlsAllocated[index] == 0)
        {
            m_values[index] = 0;
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
        m_values[index] = 0;
        s_tlsAllocated[index] = 0;
    }
}

unsigned int TLSStorageCafe::GetValue(unsigned int index)
{
    if (s_tlsAllocated[index] == 0)
    {
        return 0;
    }
    return (unsigned int)m_values[index];
}

unsigned int TLSStorageCafe::SetValue(unsigned int index, void* value)
{
    if (s_tlsAllocated[index] == 0)
    {
        return 0;
    }
    m_values[index] = value;
    return 1;
}
