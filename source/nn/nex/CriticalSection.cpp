#include "nn/nex/CriticalSection.hpp"

namespace nn
{
namespace nex
{

bool g_criticalSectionDisabled = false;

CriticalSection::CriticalSection(unsigned int traceMask)
    : m_mutex()
    , m_lockChecker(traceMask)
{
}

CriticalSection::CriticalSection(const CriticalSection& other)
    : m_mutex()
    , m_lockChecker(other.m_lockChecker)
{
}

CriticalSection::~CriticalSection()
{
}

void CriticalSection::Enter()
{
    if (g_criticalSectionDisabled)
        return;

    m_mutex.Lock();
}

void CriticalSection::Leave()
{
    if (g_criticalSectionDisabled)
        return;

    m_mutex.Unlock();
}

bool CriticalSection::IsForce() const
{
    return false;
}

ForcedCriticalSection::~ForcedCriticalSection()
{
}

void ForcedCriticalSection::Enter()
{
    m_mutex.Lock();
}

void ForcedCriticalSection::Leave()
{
    m_mutex.Unlock();
}

bool ForcedCriticalSection::IsForce() const
{
    return true;
}

}
}
