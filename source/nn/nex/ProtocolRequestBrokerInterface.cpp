#include "nn/nex/ProtocolRequestBrokerInterface.hpp"

#include <map>

#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/SecurityContextManager.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SystemError.hpp"
#include "nn/nex/ThreadVariableRoot.hpp"
#include "vendor/nn__nex.hpp"

namespace nn { namespace nex {

unsigned int (*g_protocolCurrentPID)() = 0;
unsigned int (*g_protocolCurrentCID)() = 0;

namespace
{
bool g_protocolIdentityCallbacksInitialized = false;

typedef std::map<unsigned int, Scheduler*> ReceiveSchedulerMap;
ReceiveSchedulerMap g_receiveSchedulers;
CriticalSection g_receiveSchedulerLock(0x20000000);

Scheduler* GetGlobalReceiveScheduler()
{
    Core* core = Core::GetInstance();
    return core == 0 ? 0 : core->GetScheduler();
}

void SignalReceiveSchedulerError()
{
    SystemError::SignalError(
        L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Protocol/ProtocolRequestBroker.cpp",
        72, 0xe0000003u, 0);
}
}

ProtocolRequestBrokerInterface::ProtocolRequestBrokerInterface()
    : m_reserved0(0), m_bandwidthCounters(String(L"Protocol")), m_operationManager(),
      mVTable(0x104B732Cu)
{
    if (!g_protocolIdentityCallbacksInitialized)
    {
        g_protocolIdentityCallbacksInitialized = true;
        g_protocolCurrentPID = SecurityContextManager::StaticGetCurrentPID;
        g_protocolCurrentCID = SecurityContextManager::StaticGetCurrentCID;
    }
}

ProtocolRequestBrokerInterface::~ProtocolRequestBrokerInterface()
{
}

void ProtocolRequestBrokerInterface::InitMessage(Message* message, unsigned short protocolId,
                                                 Protocol::_Command command)
{
    Protocol::AddProtocolKey(message, command, protocolId);
}

void ProtocolRequestBrokerInterface::Receive(EndPoint* endPoint, Buffer* buffer)
{
    JobProcessProtocolMessage* job =
        new JobProcessProtocolMessage(this, endPoint, buffer);

    Scheduler* scheduler = 0;
    if (g_threadSafetyEnabled)
    {
        const unsigned int threadId = ThreadVariableRoot::GetThreadID();

        g_receiveSchedulerLock.Enter();
        ReceiveSchedulerMap::iterator it = g_receiveSchedulers.find(threadId);
        if (it == g_receiveSchedulers.end())
        {
            scheduler = GetGlobalReceiveScheduler();
            g_receiveSchedulers.insert(ReceiveSchedulerMap::value_type(threadId, scheduler));
        }
        else
        {
            scheduler = it->second;
        }
        g_receiveSchedulerLock.Leave();
    }
    else
    {
        scheduler = GetGlobalReceiveScheduler();
    }

    if (scheduler == 0)
    {
        SignalReceiveSchedulerError();
        job->ReleaseRef();
        return;
    }

    scheduler->Queue(job, false);
}

}}
