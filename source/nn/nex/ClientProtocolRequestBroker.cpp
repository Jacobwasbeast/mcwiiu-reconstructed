#include "nn/nex/ClientProtocolRequestBroker.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/CriticalSection.hpp"

#include "nn/nex/CallProtocolMethodOperation.hpp"
#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/Protocol.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"

namespace nn { namespace nex {

namespace
{
enum
{
    RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u)
};

enum
{
    NO_RESPONSE_PROTOCOL_ID = 1
};
}

ClientProtocolRequestBroker::ClientProtocolRequestBroker()
    : ProtocolRequestBrokerInterface(), m_serverProtocols(), m_callPolicy()
{
    m_pCurrentEndPoint = 0;
    m_callPolicy.SetMaximumRecursionLevel(3);
}

ClientProtocolRequestBroker::~ClientProtocolRequestBroker()
{
}

Operation* ClientProtocolRequestBroker::GetCurrentOperation()
{
    return m_operationManager.GetCurrentOperation();
}

void ClientProtocolRequestBroker::ProcessMessage(CallProtocolMethodOperation* operation,
                                                  EndPoint* endPoint, Buffer* buffer)
{
    m_pCurrentEndPoint = endPoint;
    ProcessMessageCore(operation, endPoint, buffer);
    m_pCurrentEndPoint = 0;
}

void ClientProtocolRequestBroker::ProcessMessageCore(CallProtocolMethodOperation* operation,
                                                     EndPoint* endPoint, Buffer* buffer)
{
    if (operation != 0)
        m_operationManager.OperationBegins(operation);
    if (buffer == 0)
    {
        if (operation != 0)
            m_operationManager.OperationEnds(operation);
        return;
    }
    if (endPoint != 0)
        m_bandwidthCounters[endPoint->GetConnectionID()]->m_incoming += buffer->GetLength();
    Message message(buffer);
    Protocol::_Command command = Protocol::COMMAND_REQUEST;
    unsigned short protocolId = 0;
    Protocol::ExtractProtocolKey(&message, command, protocolId);
    if (command == Protocol::COMMAND_RESPONSE)
    {
        ProcessCallResponse(protocolId, &message, endPoint);
        return;
    }
    Message response;
    bool responseRequired = true;
    ProcessCallRequest(protocolId, &message, &response, &responseRequired, endPoint);
    (void)operation;
    if (endPoint != 0 && responseRequired)
    {
        Buffer* out = response.GetBuffer();
        m_bandwidthCounters[endPoint->GetConnectionID()]->m_outgoing += out->GetLength();
        endPoint->_Send(out, out->GetLength(), true, 0, 0);
    }
    if (operation != 0)
        m_operationManager.OperationEnds(operation);
}

void ClientProtocolRequestBroker::ProcessCallRequest(unsigned short protocolId, Message* request,
                                                      Message* response, bool* responseRequired,
                                                      EndPoint* endPoint)
{
    ServerProtocol* server = RVClientCore::GetInstance()->GetServerProtocol(protocolId);
    if (server == 0)
    {
        server = GetServerProtocol(protocolId);
        if (server == 0 && protocolId == NO_RESPONSE_PROTOCOL_ID)
        {
            *responseRequired = false;
            return;
        }
    }

    unsigned int callContextId = Protocol::ExtractCallContextID(request);
    Message payload;
    *responseRequired = false;
    if (server == 0)
    {
        *responseRequired = true;
        CallProtocolMethodOperation* operation =
            static_cast<CallProtocolMethodOperation*>(GetCurrentOperation());
        operation->m_result = qResult(RESULT_NOT_IMPLEMENTED);
    }
    else
    {
        DispatchCallToProtocol(server, request, &payload, responseRequired, endPoint);
    }

    if (*responseRequired)
    {
        Protocol::AddProtocolKey(response, Protocol::COMMAND_RESPONSE, protocolId);
        Protocol::AddCallResponse(response,
                                  static_cast<CallProtocolMethodOperation*>(GetCurrentOperation()),
                                  callContextId, payload);
    }
}

void ClientProtocolRequestBroker::ProcessCallResponse(unsigned short protocolId, Message* response,
                                                       EndPoint* endPoint)
{
    ClientProtocol* client = static_cast<ClientProtocol*>(
        RVClientCore::GetInstance()->GetProtocolRegistry()->GetProtocol(protocolId,
                                                                        Protocol::TYPE_CLIENT));
    if (client != 0)
        client->ProcessResponse(response, endPoint);
}

bool ClientProtocolRequestBroker::RegisterProtocol(Protocol* protocol)
{
    return false;
}

void ClientProtocolRequestBroker::UnregisterProtocol(Protocol* protocol)
{
    (void)protocol;
}

Protocol* ClientProtocolRequestBroker::GetProtocol(unsigned short protocolId, Protocol::_Type type)
{
    (void)protocolId; (void)type;
    return 0;
}

bool ClientProtocolRequestBroker::CloneProtocols()
{
    return false;
}

bool ClientProtocolRequestBroker::RegisterServerProtocol(unsigned short protocolId,
                                                          ServerProtocol* protocol)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_serverProtocols.find(protocolId) != m_serverProtocols.end())
        return false;
    m_serverProtocols[protocolId] = protocol;
    return true;
}

bool ClientProtocolRequestBroker::UnregisterServerProtocol(unsigned short protocolId)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    ServerProtocolMap::iterator entry = m_serverProtocols.find(protocolId);
    if (entry != m_serverProtocols.end())
        m_serverProtocols.erase(entry);
    return true;
}

ServerProtocol* ClientProtocolRequestBroker::GetServerProtocol(unsigned short protocolId)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    ServerProtocolMap::iterator entry = m_serverProtocols.find(protocolId);
    if (entry == m_serverProtocols.end())
        return 0;
    return entry->second;
}

qResult ClientProtocolRequestBroker::DispatchLocalRMC(Message* message, ClientProtocol* client,
                                                       ServerProtocol* server, EndPoint* endPoint)
{
    return qResult(RESULT_NOT_IMPLEMENTED);
}

qResult ClientProtocolRequestBroker::DispatchBlockingLocalRMC(Message* message,
                                                               ClientProtocol* client,
                                                               ServerProtocol* server,
                                                               EndPoint* endPoint)
{
    return qResult(RESULT_NOT_IMPLEMENTED);
}

qResult ClientProtocolRequestBroker::DispatchThreadSafeLocalRMC(Message* message,
                                                                 ClientProtocol* client,
                                                                 ServerProtocol* server,
                                                                 EndPoint* endPoint)
{
    return qResult(RESULT_NOT_IMPLEMENTED);
}

void ClientProtocolRequestBroker::PeerDisconnected(EndPoint* endPoint)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    Core::GetInstance()->GetCallContextRegister()->CancelCallContexts(endPoint,
                                                                     endPoint->GetConnectionID());
}

void ClientProtocolRequestBroker::FaultDetection(EndPoint* endPoint, unsigned int reason)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    Core::GetInstance()->GetCallContextRegister()->CancelCallContexts(endPoint,
                                                                     endPoint->GetConnectionID());
}

void ClientProtocolRequestBroker::DispatchCallToProtocol(ServerProtocol* protocol, Message* request,
                                                          Message* response, bool* responseRequired,
                                                          EndPoint* endPoint)
{
    CallPolicy& policy = m_callPolicy;
    policy.BeginCallImpl();
    int attempt = 0;
    protocol->DispatchCallRequest(
        static_cast<CallProtocolMethodOperation*>(GetCurrentOperation())->GetAttemptCount(),
        request, response, responseRequired, &attempt, endPoint);
    policy.EndCallImpl();
}

}}
