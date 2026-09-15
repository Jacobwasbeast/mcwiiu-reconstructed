#include "nn/pia/common/CachedPrint.hpp"

#include <cstdio>
#include <new>

namespace nn { namespace pia { namespace common {

CachedPrint* CachedPrint::s_instance = 0;
void* CachedPrint::s_storage = 0;

CachedPrint::CachedPrint(unsigned int capacity)
    : m_buffer(0), m_capacity(capacity), m_start(0), m_position(0), m_end(0),
      m_criticalSection(7)
{
    m_buffer = static_cast<char*>(pead::allocateFromSeadHeap(
        capacity, HeapManager::GetHeap()));
    for (unsigned int index = 0; index < capacity; ++index)
        if (m_buffer != 0)
            m_buffer[index] = '\0';
}

CachedPrint::~CachedPrint()
{
    if (m_buffer != 0)
    {
        pead::getArraySize(m_buffer);
        pead::freeToSeadHeap(m_buffer);
    }
}

Result CachedPrint::CreateInstance(unsigned int capacity)
{
    static const Result kNotInitialized = 0x10c0c;
    static const Result kInvalidState = 0x10c08;
    static const Result kInvalidCapacity = 0x10c07;
    static const Result kAllocationFailed = 0x10c01;
    static const Result kAlreadyCreated = 0x10c0f;

    if (IsInitialized() == 0)
        return kNotInitialized;
    if (IsInSetupMode() == 0)
        return kInvalidState;
    if (s_instance != 0)
        return kAlreadyCreated;
    if (capacity < 0x200)
        return kInvalidCapacity;

    s_storage = pead::allocateFromSeadHeap(0x40, HeapManager::GetHeap(), 8);
    if (s_storage == 0)
        return kAllocationFailed;
    unsigned char* storageBytes = static_cast<unsigned char*>(s_storage);
    for (unsigned int index = 0; index < 0x40; ++index)
        if (storageBytes != 0)
            storageBytes[index] = 0;
    s_instance = new (s_storage) CachedPrint(capacity);
    return 0;
}

void CachedPrint::DestroyInstance()
{
    if (s_instance != 0)
    {
        s_instance->~CachedPrint();
        s_instance = 0;
    }
    if (s_storage != 0)
    {
        pead::getArraySize(s_storage);
        pead::freeToSeadHeap(s_storage);
        s_storage = 0;
    }
}

void CachedPrint::VPrintf(const char* format, va_list args)
{
    m_criticalSection.Lock();
    unsigned int maxLength = m_capacity - m_position;
    if (maxLength < 0x100)
    {
        maxLength = m_capacity;
        if (m_end != m_position || m_start == 0)
            m_start = 1;
        m_position = 0;
    }
    int written = std::vsnprintf(m_buffer + m_position, maxLength, format, args);
    if (written < 0 || maxLength <= static_cast<unsigned int>(written))
    {
        written = static_cast<int>(maxLength - 1);
        m_buffer[m_capacity - 1] = 0;
    }
    const unsigned int newPosition = m_position + static_cast<unsigned int>(written);
    if (m_end == m_position)
    {
        m_position = newPosition;
        m_end = newPosition;
    }
    else if (newPosition < m_end)
    {
        m_position = newPosition;
        if (m_start <= newPosition)
            m_start = newPosition + 1;
    }
    else
    {
        m_position = newPosition;
        m_end = newPosition;
        m_start = 0;
    }
    m_criticalSection.Unlock();
}

void CachedPrint::Printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    VPrintf(format, args);
    va_end(args);
}

void CachedPrint::Clear()
{
    m_criticalSection.Lock();
    m_end = 0;
    m_start = 0;
    m_position = 0;
    m_criticalSection.Unlock();
}

void CachedPrint::Flush()
{
    m_criticalSection.Lock();
    const unsigned int start = m_start;
    unsigned int end = m_end;
    if (end != start)
    {
        pead::system::PrintString(m_buffer + start, static_cast<int>(end - start));
        end = m_end;
    }
    if (end != m_position && m_position != 0)
        pead::system::PrintString(m_buffer, static_cast<int>(m_position));
    Clear();
    m_criticalSection.Unlock();
}

}}}
