#include "nn/nex/PacketBufferMultiUnitHeap.hpp"

#include "nn/nex/CriticalSection.hpp"

namespace nn
{
namespace nex
{

bool g_threadSafetyEnabled = true;

void PacketBufferMultiUnitHeap::Lock() const
{
    if (m_pMutex == 0)
        return;

    if (g_criticalSectionDisabled)
        return;

    m_pMutex->Lock();
}

void PacketBufferMultiUnitHeap::Unlock() const
{
    if (m_pMutex == 0)
        return;

    if (g_criticalSectionDisabled)
        return;

    m_pMutex->Unlock();
}

void PacketBufferMultiUnitHeap::Initialize(void* pMemory, unsigned int,
    qVector<PacketBufferManager::UnitHeapParam> aParams)
{
    unsigned char* pCursor = static_cast<unsigned char*>(pMemory);

    m_uiUnitHeapNum = static_cast<unsigned int>(aParams.size());
    m_pUnitHeaps = new PacketBufferUnitHeap[m_uiUnitHeapNum];

    for (unsigned int i = 0; i < m_uiUnitHeapNum; ++i)
    {
        PacketBufferUnitHeap& rUnitHeap = m_pUnitHeaps[i];

        rUnitHeap.m_uiUnitSize = aParams[i].m_uiUnitSize;

        const unsigned int uiBlockSize = (aParams[i].m_uiUnitSize + 11) & ~3u;
        const unsigned int uiTotalSize = uiBlockSize * aParams[i].m_uiUnitNum;
        const unsigned int uiUnitNum = uiTotalSize / uiBlockSize;

        rUnitHeap.m_pMemory = pCursor;
        rUnitHeap.m_uiTotalSize = uiTotalSize;
        rUnitHeap.m_uiUsedUnitNum = 0;
        rUnitHeap.m_pFreeList = 0;

        unsigned char* pBlock = pCursor;

        for (unsigned int j = 0; j < uiUnitNum; ++j)
        {
            PacketBufferUnitHeap::Unit* pUnit =
                reinterpret_cast<PacketBufferUnitHeap::Unit*>(pBlock);

            pUnit->m_pNext = rUnitHeap.m_pFreeList;
            rUnitHeap.m_pFreeList = pUnit;
            pBlock += uiBlockSize;
        }

        rUnitHeap.ResetDebugInfo();

        if (m_uiMaxUnitSize < aParams[i].m_uiUnitSize)
            m_uiMaxUnitSize = aParams[i].m_uiUnitSize;

        pCursor += uiTotalSize;
    }

    m_bUnitExhausted = false;
    m_bInitialized = true;

    if (g_threadSafetyEnabled)
        m_pMutex = new MutexPrimitive();
}

void PacketBufferMultiUnitHeap::Finalize()
{
    m_bInitialized = false;

    for (unsigned int i = 0; i < m_uiUnitHeapNum; ++i)
        m_pUnitHeaps[i].m_pFreeList = 0;

    delete[] m_pUnitHeaps;
    m_pUnitHeaps = 0;

    if (m_pMutex != 0)
    {
        delete m_pMutex;
        m_pMutex = 0;
    }
}

void* PacketBufferMultiUnitHeap::Allocate(unsigned int uiSize,
    MemoryManager::Deallocator pDeallocator)
{
    void* pBlock = 0;

    Lock();

    for (unsigned int i = 0; i < m_uiUnitHeapNum; ++i)
    {
        PacketBufferUnitHeap& rUnitHeap = m_pUnitHeaps[i];

        if (uiSize > rUnitHeap.m_uiUnitSize)
            continue;

        PacketBufferUnitHeap::Unit* pUnit = rUnitHeap.m_pFreeList;

        if (pUnit == 0)
        {
            ++rUnitHeap.m_debugInfo.m_uiAllocFailedNum;
            continue;
        }

        rUnitHeap.m_pFreeList = pUnit->m_pNext;

        ++rUnitHeap.m_debugInfo.m_uiUsedUnitNum;

        if (rUnitHeap.m_debugInfo.m_uiMaxUsedUnitNum < rUnitHeap.m_debugInfo.m_uiUsedUnitNum)
            rUnitHeap.m_debugInfo.m_uiMaxUsedUnitNum = rUnitHeap.m_debugInfo.m_uiUsedUnitNum;

        ++rUnitHeap.m_debugInfo.m_uiAllocNum;
        ++rUnitHeap.m_uiUsedUnitNum;

        pBlock = MemoryManager::AllocateForPbPool(pUnit, pDeallocator, &rUnitHeap);
        m_bUnitExhausted = false;
        break;
    }

    if (pBlock == 0)
    {
        if (uiSize > m_uiMaxUnitSize)
            pBlock = MemoryManager::Allocate(uiSize);
        else
            m_bUnitExhausted = true;
    }

    Unlock();
    return pBlock;
}

void PacketBufferMultiUnitHeap::Free(void* pBlock)
{
    Lock();

    PacketBufferUnitHeap::Unit* pUnit = static_cast<PacketBufferUnitHeap::Unit*>(pBlock);
    PacketBufferUnitHeap* pUnitHeap = pUnit->m_pUnitHeap;

    pUnit->m_pNext = pUnitHeap->m_pFreeList;
    ++pUnitHeap->m_debugInfo.m_uiFreeNum;
    pUnitHeap->m_pFreeList = pUnit;
    --pUnitHeap->m_uiUsedUnitNum;
    --pUnitHeap->m_debugInfo.m_uiUsedUnitNum;

    Unlock();
}

void PacketBufferMultiUnitHeap::GetDebugInfo(
    qVector<PacketBufferManager::UnitHeapDebugInfo>& rDebugInfo) const
{
    Lock();

    for (unsigned int i = 0; i < m_uiUnitHeapNum; ++i)
        rDebugInfo.push_back(m_pUnitHeaps[i].m_debugInfo);

    Unlock();
}

}
}
