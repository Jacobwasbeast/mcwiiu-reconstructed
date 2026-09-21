#include "nn/nex/PacketBufferManager.hpp"

#include <algorithm>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/GlobalVariables.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/PacketBufferMultiUnitHeap.hpp"

namespace nn
{
namespace nex
{

namespace
{

CriticalSection s_criticalSection(0x20000000);

const unsigned int PACKET_MANAGEMENT_OBJECT_SIZES[3] = { 40, 36, 328 };

const unsigned int PACKET_MANAGEMENT_OBJECT_KINDS = 3;

const unsigned int PACKET_MANAGEMENT_OBJECT_MARGIN = 4;

const unsigned int PACKET_BUFFER_HEADER_MARGIN = 0x50;

}

bool PacketBufferManager::IsEnablePacketBufferPool()
{
    s_criticalSection.Enter();
    const bool enabled = g_globalVariables->m_packetBufferPoolEnabled;
    s_criticalSection.Leave();
    return enabled;
}

void PacketBufferManager::GetUnitHeapParam(qVector<UnitHeapParam>& rParams)
{
    s_criticalSection.Enter();
    if (!g_globalVariables->m_packetBufferParams.empty())
    {
        rParams = g_globalVariables->m_packetBufferParams;
    }
    else
    {
        qVector<UnitHeapParam> aParams;
        GetSimpleUnitHeapParam(0x4b000, 0x554, aParams);
        rParams = aParams;
    }
    s_criticalSection.Leave();
}

bool PacketBufferManager::IsActiveNonStatic()
{
    return m_bActive;
}

bool PacketBufferManager::IsActive()
{
    s_criticalSection.Enter();

    bool bActive = false;
    PacketBufferManager* pInstance = GetInstance();

    if (pInstance != 0 && pInstance->IsActiveNonStatic())
        bActive = true;

    s_criticalSection.Leave();
    return bActive;
}

unsigned int PacketBufferManager::GetRequiredMemorySizeImpl(const qVector<UnitHeapParam>& rParams)
{
    unsigned int uiSize = 0;

    for (unsigned int i = 0; i < rParams.size(); ++i)
        uiSize += ((rParams[i].m_uiUnitSize + 11) & ~3u) * rParams[i].m_uiUnitNum;

    return uiSize;
}

bool PacketBufferManager::CheckUnitHeapParam(const qVector<UnitHeapParam>& rParams)
{
    if (rParams.size() == 0)
        return true;

    bool bHasMaxUnit = false;

    for (unsigned int i = 0; i < rParams.size(); ++i)
    {
        const unsigned int uiUnitSize = rParams[i].m_uiUnitSize;

        if ((uiUnitSize & 3) != 0)
            return false;

        if (uiUnitSize == 0)
            return false;

        if (uiUnitSize > MaxUnitSize)
            return false;

        if (rParams[i].m_uiUnitNum == 0)
            return false;

        if (uiUnitSize == MaxUnitSize)
            bHasMaxUnit = true;
    }

    if (!bHasMaxUnit)
        return false;

    return true;
}

void PacketBufferManager::GetPacketManagementUnitHeapParam(
    qVector<UnitHeapParam>& rManagementParams, const qVector<UnitHeapParam>& rParams)
{
    unsigned int uiUnitNum = 0;

    for (unsigned int i = 0; i < rParams.size(); ++i)
        uiUnitNum += rParams[i].m_uiUnitNum;

    qVector<UnitHeapParam> aParams;

    for (unsigned int i = 0; i < PACKET_MANAGEMENT_OBJECT_KINDS; ++i)
    {
        UnitHeapParam param;

        param.m_uiUnitNum = uiUnitNum + PACKET_MANAGEMENT_OBJECT_MARGIN;
        param.m_uiUnitSize = PACKET_MANAGEMENT_OBJECT_SIZES[i];
        aParams.push_back(param);
    }

    std::sort(aParams.begin(), aParams.end());
    rManagementParams = aParams;
}

unsigned int PacketBufferManager::GetRequiredMemorySize(const qVector<UnitHeapParam>& rParams)
{
    if (!CheckUnitHeapParam(rParams))
        return 0;

    unsigned int uiSize = GetRequiredMemorySizeImpl(rParams);

    if (uiSize != 0)
    {
        qVector<UnitHeapParam> aManagementParams;

        GetPacketManagementUnitHeapParam(aManagementParams, rParams);
        uiSize += GetRequiredMemorySizeImpl(aManagementParams);
    }

    return uiSize;
}

bool PacketBufferManager::GetSimpleUnitHeapParam(unsigned int uiPoolSize, unsigned int uiUnitSize,
    qVector<UnitHeapParam>& rParams)
{
    if (uiPoolSize < MinimumPoolSize || uiUnitSize == 0 || uiUnitSize > MaxUnitSize)
        return false;

    uiUnitSize += PACKET_BUFFER_HEADER_MARGIN;

    if (uiUnitSize > MaxUnitSize)
        uiUnitSize = MaxUnitSize;

    const unsigned int uiAlignedUnitSize = (uiUnitSize + 3) & ~3u;

    qVector<UnitHeapParam> aParams;

    if (uiAlignedUnitSize < MaxUnitSize)
    {
        UnitHeapParam param;

        param.m_uiUnitSize = uiAlignedUnitSize;
        param.m_uiUnitNum = 0;
        aParams.push_back(param);
    }

    UnitHeapParam maxParam;

    maxParam.m_uiUnitSize = MaxUnitSize;
    maxParam.m_uiUnitNum = 0;
    aParams.push_back(maxParam);

    unsigned int uiManagementBlockSize = 0;

    for (unsigned int i = 0; i < PACKET_MANAGEMENT_OBJECT_KINDS; ++i)
        uiManagementBlockSize += (PACKET_MANAGEMENT_OBJECT_SIZES[i] + 11) & ~3u;

    for (unsigned int i = 0; i < aParams.size(); ++i)
    {
        const unsigned int uiUnitNum =
            ((uiPoolSize - uiManagementBlockSize * PACKET_MANAGEMENT_OBJECT_MARGIN)
                / static_cast<unsigned int>(aParams.size()))
            / (((aParams[i].m_uiUnitSize + 11) & ~3u) + uiManagementBlockSize);

        if (uiUnitNum == 0)
            return false;

        aParams[i].m_uiUnitNum = uiUnitNum;
    }

    std::sort(aParams.begin(), aParams.end());
    rParams = aParams;
    return true;
}

void* PacketBufferManager::PacketBufferAllocate(unsigned int uiSize)
{
    return GetInstance()->m_pPacketBufferHeap->Allocate(uiSize, PacketBufferFree);
}

void PacketBufferManager::PacketBufferFree(void* pBlock)
{
    if (pBlock == 0)
        return;

    GetInstance()->m_pPacketBufferHeap->Free(pBlock);
}

void* PacketBufferManager::PacketManagementObjectAllocate(unsigned int uiSize)
{
    return GetInstance()->m_pManagementObjectHeap->Allocate(uiSize, PacketManagementObjectFree);
}

void PacketBufferManager::PacketManagementObjectFree(void* pBlock)
{
    if (pBlock == 0)
        return;

    GetInstance()->m_pManagementObjectHeap->Free(pBlock);
}

unsigned int PacketBufferManager::GetMaxUnitSize()
{
    if (m_pPacketBufferHeap == 0)
        return 0;

    return m_pPacketBufferHeap->m_uiMaxUnitSize;
}

unsigned int PacketBufferManager::GetProperUnitSize(unsigned int uiSize)
{
    PacketBufferMultiUnitHeap* pHeap = m_pPacketBufferHeap;

    if (pHeap != 0 && uiSize <= pHeap->m_uiMaxUnitSize)
    {
        for (unsigned int i = 0; i < pHeap->m_uiUnitHeapNum; ++i)
        {
            if (uiSize <= pHeap->m_pUnitHeaps[i].m_uiUnitSize)
                return pHeap->m_pUnitHeaps[i].m_uiUnitSize;
        }
    }

    return 0;
}

void* PacketBufferManager::Allocate(unsigned int uiSize)
{
    if (m_pfnAllocate != 0)
        return m_pfnAllocate(uiSize);

    return MemoryManager::Allocate(uiSize);
}

void* PacketBufferManager::AllocateManagementObject(unsigned int uiSize)
{
    if (m_pfnAllocateManagementObject != 0)
        return m_pfnAllocateManagementObject(uiSize);

    return MemoryManager::Allocate(uiSize);
}

bool PacketBufferManager::IsAllocatable(unsigned int uiSize, unsigned int uiUnitNum)
{
    PacketBufferMultiUnitHeap* pHeap = m_pPacketBufferHeap;

    if (pHeap == 0)
        return true;

    if (uiSize > pHeap->m_uiMaxUnitSize)
        return true;

    bool bAllocatable = false;
    unsigned int uiFreeUnitNum = 0;

    pHeap->Lock();

    for (unsigned int i = 0; i < pHeap->m_uiUnitHeapNum; ++i)
    {
        PacketBufferUnitHeap& rUnitHeap = pHeap->m_pUnitHeaps[i];

        if (uiSize > rUnitHeap.m_uiUnitSize)
            continue;

        uiFreeUnitNum += rUnitHeap.m_uiTotalSize / ((rUnitHeap.m_uiUnitSize + 11) & ~3u)
            - rUnitHeap.m_uiUsedUnitNum;

        if (uiUnitNum <= uiFreeUnitNum)
        {
            bAllocatable = true;
            pHeap->m_bUnitExhausted = false;
            pHeap->Unlock();
            return bAllocatable;
        }
    }

    pHeap->m_bUnitExhausted = true;
    ++pHeap->m_pUnitHeaps[pHeap->m_uiUnitHeapNum - 1].m_debugInfo.m_uiAllocFailedNum;

    pHeap->Unlock();
    return bAllocatable;
}

void PacketBufferManager::GetUnitHeapDebugInfo(qVector<UnitHeapDebugInfo>& rDebugInfo)
{
    s_criticalSection.Enter();

    rDebugInfo.clear();

    PacketBufferManager* pInstance = GetInstance();

    if (pInstance != 0 && pInstance->m_pPacketBufferHeap != 0)
        pInstance->m_pPacketBufferHeap->GetDebugInfo(rDebugInfo);

    s_criticalSection.Leave();
}

void PacketBufferManager::GetAllUnitHeapDebugInfo(qVector<UnitHeapDebugInfo>& rDebugInfo)
{
    s_criticalSection.Enter();

    PacketBufferManager* pInstance = GetInstance();

    qVector<UnitHeapDebugInfo> aPacketBufferDebugInfo;

    if (pInstance != 0 && pInstance->m_pPacketBufferHeap != 0)
        pInstance->m_pPacketBufferHeap->GetDebugInfo(aPacketBufferDebugInfo);

    qVector<UnitHeapDebugInfo> aManagementObjectDebugInfo;

    if (pInstance != 0 && pInstance->m_pManagementObjectHeap != 0)
        pInstance->m_pManagementObjectHeap->GetDebugInfo(aManagementObjectDebugInfo);

    rDebugInfo.clear();
    rDebugInfo.insert(rDebugInfo.end(), aPacketBufferDebugInfo.begin(),
        aPacketBufferDebugInfo.end());
    rDebugInfo.insert(rDebugInfo.end(), aManagementObjectDebugInfo.begin(),
        aManagementObjectDebugInfo.end());

    s_criticalSection.Leave();
}

void PacketBufferManager::Dump(unsigned long long)
{
    if (m_pPacketBufferHeap != 0)
    {
        qVector<UnitHeapDebugInfo> aDebugInfo;

        m_pPacketBufferHeap->GetDebugInfo(aDebugInfo);
    }

    if (m_pManagementObjectHeap != 0)
    {
        qVector<UnitHeapDebugInfo> aDebugInfo;

        m_pManagementObjectHeap->GetDebugInfo(aDebugInfo);
    }
}

void PacketBufferManager::Initialize()
{
    s_criticalSection.Enter();
    if (m_iInitializeCount == 0 && IsEnablePacketBufferPool() && !IsActive())
    {
        qVector<UnitHeapParam> aParams;
        GetUnitHeapParam(aParams);
        const unsigned int uiMemorySize = GetRequiredMemorySize(aParams);
        m_pMemory = MemoryManager::Allocate(uiMemorySize);
        m_uiMemorySize = uiMemorySize;

        PacketBufferMultiUnitHeap* pPacketHeap = new PacketBufferMultiUnitHeap();
        const unsigned int uiPacketMemorySize = GetRequiredMemorySizeImpl(aParams);
        pPacketHeap->Initialize(m_pMemory, uiPacketMemorySize, aParams);
        m_pPacketBufferHeap = pPacketHeap;

        {
            qVector<UnitHeapParam> aManagementParams;
            GetPacketManagementUnitHeapParam(aManagementParams, aParams);
            PacketBufferMultiUnitHeap* pManagementHeap = new PacketBufferMultiUnitHeap();
            const unsigned int uiManagementMemorySize = GetRequiredMemorySizeImpl(aManagementParams);
            pManagementHeap->Initialize(static_cast<unsigned char*>(m_pMemory) + uiPacketMemorySize,
                                       uiManagementMemorySize, aManagementParams);
            m_pManagementObjectHeap = pManagementHeap;
        }
        m_bActive = true;
        m_pfnAllocate = PacketBufferAllocate;
        m_pfnAllocateManagementObject = PacketManagementObjectAllocate;
    }
    m_iInitializeCount = static_cast<int>(static_cast<unsigned int>(m_iInitializeCount) + 1u);
    s_criticalSection.Leave();
}

void PacketBufferManager::Finalize()
{
    s_criticalSection.Enter();
    --m_iInitializeCount;
    s_criticalSection.Leave();
}

}
}
