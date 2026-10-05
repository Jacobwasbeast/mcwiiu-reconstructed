#include "nn/nex/TransportBufferSet.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/SpecialArray.hpp"
#include "nn/nex/TransportBuffer.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int DEFAULT_BUFFER_SIZE = 1364;
const unsigned int TRACE_MASK = 0x100000;

}

TransportBufferSet::TransportBufferSet(TransportBuffer* pTransportBuffer, int iCount)
    : m_useInfos(iCount), m_freeInfos(iCount), m_uiBufferSize(DEFAULT_BUFFER_SIZE),
      m_criticalSection(TRACE_MASK), m_uiEmptyDropCount(0), m_pTransportBuffer(pTransportBuffer)
{
    m_bHasUse = false;
    m_bFull = false;
    for (int i = 0; i < iCount; ++i)
        m_freeInfos.push(SpecialNew<TransportBufferInfo>(0, 0));
}

TransportBufferSet::~TransportBufferSet()
{
    while (!m_freeInfos.empty())
    {
        SpecialDelete(m_freeInfos.front());
        m_freeInfos.pop();
    }
}

TransportBufferInfo* TransportBufferSet::PopNewSetInfo()
{
    if (m_freeInfos.empty())
    {
        ++m_uiEmptyDropCount;
        return 0;
    }
    Buffer* pBuffer;
    if (m_pTransportBuffer == 0)
        pBuffer = new Buffer(m_uiBufferSize, 0);
    else
        pBuffer = m_pTransportBuffer->AllocateBuffer(m_uiBufferSize);
    if (pBuffer != 0)
    {
        m_bHasUse = true;
        return CreateInfo_(pBuffer);
    }
    return 0;
}

TransportBufferInfo* TransportBufferSet::CreateInfo_(Buffer* pBuffer)
{
    TransportBufferInfo* pInfo = m_freeInfos.front();
    m_freeInfos.pop();
    m_useInfos.push(pInfo);
    pInfo->m_pBuffer = pBuffer;
    return pInfo;
}

TransportBufferInfo* TransportBufferSet::PopNewUseInfo()
{
    CriticalSection* pLock = &m_criticalSection;
    pLock->Enter();
    TransportBufferInfo* pInfo = 0;
    if (!m_useInfos.empty())
    {
        pInfo = m_useInfos.front();
        m_useInfos.pop();
    }
    bool bHasUse = m_useInfos.size() != 0;
    bool bFull = m_useInfos.size() == static_cast<int>(m_useInfos.capacity());
    m_bHasUse = bHasUse;
    m_bFull = bFull;
    pLock->Leave();
    return pInfo;
}

void TransportBufferSet::DeleteOneBuffer_(TransportBufferInfo* pInfo)
{
    if (pInfo != 0)
    {
        if (pInfo->m_pBuffer != 0)
        {
            if (m_pTransportBuffer != 0)
            {
                m_pTransportBuffer->FreeBuffer(pInfo->m_pBuffer);
                m_freeInfos.push(pInfo);
                return;
            }
            delete pInfo->m_pBuffer;
        }
        m_freeInfos.push(pInfo);
    }
}

void TransportBufferSet::FreeOneBuffer(TransportBufferInfo* pInfo)
{
    CriticalSection* pLock = &m_criticalSection;
    pLock->Enter();
    DeleteOneBuffer_(pInfo);
    pLock->Leave();
}

void TransportBufferSet::ClearList_(qReservedQueue<TransportBufferInfo*>& rQueue)
{
    while (!rQueue.empty())
    {
        DeleteOneBuffer_(rQueue.front());
        rQueue.pop();
    }
}

}
}
