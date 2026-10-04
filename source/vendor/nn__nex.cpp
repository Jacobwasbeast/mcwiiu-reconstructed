#include "vendor/nn__nex.hpp"

#include "nn/nex/BadEvents.hpp"
#include "nn/nex/Buffer.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/HighResolutionClock.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/String.hpp"

namespace nn { namespace nex {

BadEvents* BadEvents::s_pInstance = NULL;

bool g_jobProcessProtocolEventCloseGuard = false;

JobProcessProtocolEvent::JobProcessProtocolEvent(ProtocolRequestBrokerInterface* pBroker,
                                                 EndPoint* pEndPoint,
                                                 const wchar_t* pName)
    : Job(JOB_TYPE_2, DebugString(String(pName)))
    , m_pBroker(pBroker)
    , m_pEndPoint(pEndPoint)
    , m_lifeTime(0)
    , m_processingStart(0)
    , m_processingTime(0)
{
    if (!g_jobProcessProtocolEventCloseGuard)
    {
        m_pEndPoint->Open();
        m_lifeTime = HighResolutionClock::GetHighResolutionTime();
    }
}

JobProcessProtocolEvent::~JobProcessProtocolEvent()
{
    if (!g_jobProcessProtocolEventCloseGuard)
        m_pEndPoint->Close();
}

void JobProcessProtocolEvent::Trace(uint64_t)
{
}

JobProcessProtocolMessage::JobProcessProtocolMessage(ProtocolRequestBrokerInterface* pBroker,
                                                     EndPoint* pEndPoint,
                                                     Buffer* pBuffer)
    : JobProcessProtocolEvent(pBroker, pEndPoint, L"ProcessProtocoMessage")
    , m_operation()
    , m_pBuffer(pBuffer)
{
    m_pBuffer->AddRef();
}

JobProcessProtocolMessage::~JobProcessProtocolMessage()
{
    m_pBuffer->ReleaseRef();
}

void JobProcessProtocolMessage::Execute()
{
    m_pBroker->ProcessMessage(&m_operation, m_pEndPoint, m_pBuffer);

    if (m_operation.m_postponed)
        SetToWaiting(m_operation.m_postponementDelay);
    else
        SetToComplete();
}

void JobProcessProtocolMessage::Trace(uint64_t)
{
}

void BadEvents::Signal(_ID id)
{
    if (s_pInstance == NULL)
        return;
    s_pInstance->SignalEvent(id);
}

void BadEvents::SignalEvent(_ID id)
{
    ++m_eventCounts[id];
}

}}
