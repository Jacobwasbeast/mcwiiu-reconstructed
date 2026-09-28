#include "vendor/nn__nex.hpp"

#include <cstring>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/HighResolutionClock.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/String.hpp"

namespace nn { namespace nex {

class BadEvents
{
public:
    enum Event
    {
        SYSTEM_CHECK_FAILURE = 0
    };

    static void Signal(Event event);
};

bool g_jobProcessProtocolEventCloseGuard = false;

namespace
{
bool OperationPostponed(const CallProtocolMethodOperation& operation)
{
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&operation);
    return bytes[0x18] != 0;
}

int OperationWaitContext(const CallProtocolMethodOperation& operation)
{
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&operation);
    int result = 0;
    std::memcpy(&result, bytes + 0x1C, sizeof(result));
    return result;
}
}

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

    if (OperationPostponed(m_operation))
        SetToWaiting(OperationWaitContext(m_operation));
    else
        SetToComplete();
}

void JobProcessProtocolMessage::Trace(uint64_t)
{
}

void BadEvents::Signal(Event event)
{
    (void)event;
}

}}
