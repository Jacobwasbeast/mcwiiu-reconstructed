#include "nn/nex/ProtocolRequestBroker.hpp"

#include "nn/nex/CallProtocolMethodOperation.hpp"
#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/SingleThreadCallPolicy.hpp"
#include "nn/nex/SecurityContextManager.hpp"
#include "nn/nex/LogToClientController.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StringStream.hpp"
#include <map>
#include <vector>

namespace nn { namespace nex {

namespace
{
struct BrokerState
{
    BrokerState() : registry(), policy(), protocols() { policy.SetMaximumRecursionLevel(3); }
    ProtocolRegistry registry;
    SingleThreadCallPolicy policy;
    std::vector<Protocol*> protocols;
};
typedef std::map<const ProtocolRequestBroker*, BrokerState*> BrokerStateMap;
enum
{
    PROTOCOL_FLAG_THREAD_SAFE_RMC = 2
};
BrokerStateMap g_brokerStates;
BrokerState* StateFor(const ProtocolRequestBroker* broker)
{
    BrokerStateMap::iterator it = g_brokerStates.find(broker);
    if (it != g_brokerStates.end()) return it->second;
    BrokerState* state = new BrokerState();
    g_brokerStates[broker] = state;
    return state;
}
}

ProtocolRequestBroker::~ProtocolRequestBroker()
{
    BrokerStateMap::iterator it = g_brokerStates.find(this);
    if (it != g_brokerStates.end())
    {
        delete it->second;
        g_brokerStates.erase(it);
    }
}

ProtocolRegistry& ProtocolRequestBroker::GetRegistry()
{
    return StateFor(this)->registry;
}

Operation* ProtocolRequestBroker::GetCurrentOperation()
{
    return m_operationManager.GetCurrentOperation();
}

void ProtocolRequestBroker::ProcessMessage(CallProtocolMethodOperation* operation,
                                            EndPoint* endPoint, Buffer* buffer)
{
    m_pCurrentEndPoint = endPoint;
    ProcessMessageCore(operation, endPoint, buffer);
    m_pCurrentEndPoint = 0;
}

void ProtocolRequestBroker::ProcessMessageCore(CallProtocolMethodOperation* operation,
                                               EndPoint* endPoint, Buffer* buffer)
{
    Message message(buffer);
    Protocol::_Command command;
    unsigned short protocolId;
    Protocol::ExtractProtocolKey(&message, command, protocolId);
    m_bandwidthCounters[protocolId]->m_incoming += buffer->GetLength();
    if (command == Protocol::COMMAND_REQUEST)
    {
        operation->InitializeForNewCallAttempt();
        m_operationManager.OperationBegins(operation);
        Message response;
        bool responseRequired = false;
        LogToClientController::Accumulate();
        ProcessCallRequest(protocolId, &message, &response, &responseRequired, endPoint);
        LogToClientController::Flush(endPoint);
        if (!operation->m_postponed && responseRequired)
        {
            Buffer* out = response.GetBuffer();
            {
                ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
                endPoint->_Send(out, 1, false, 0, 0);
            }
            m_bandwidthCounters[protocolId]->m_outgoing += out->GetLength();
        }
        m_operationManager.OperationEnds(operation);
    }
    else
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        if (m_pParentBroker == 0)
        {
            ProcessCallResponse(protocolId, &message, endPoint);
        }
        else
        {
            m_pParentBroker->ProcessCallResponse(protocolId, &message, endPoint);
        }
    }

    if (message.HasError())
    {
        StringStream stream;
        stream << L"ProtocolRequestBroker: Error processing message. pBuffer=" << buffer
               << L" ProtocolID=";
        stream.StreamNumber(protocolId) << L" CntSize=";
        stream.StreamNumber(buffer->GetLength()) << L" Size=";
        stream.StreamNumber(buffer->GetRealSize() - buffer->GetHeadShiftSize());
        if (endPoint != 0)
        {
            stream << L"  EndPoint: CID=";
            stream.StreamNumber(endPoint->m_uiConnectionID) << L" PID=";
            stream.StreamNumber(endPoint->m_uiPrincipalID) << L" URL=" << endPoint->m_stationURL;
        }
        else
        {
            stream << L"  <EndPoint is NULL>";
        }
    }
}

void ProtocolRequestBroker::ProcessCallRequest(unsigned short protocolId, Message* request,
                                               Message* response, bool* responseRequired,
                                               EndPoint* endPoint)
{
    ProcessLocalCallRequest(GetServerProtocol(protocolId), request, response,
                            responseRequired, endPoint);
}

void ProtocolRequestBroker::ProcessLocalCallRequest(ServerProtocol* protocol, Message* request,
                                                    Message* response, bool* responseRequired,
                                                    EndPoint* endPoint)
{
    if (protocol == 0)
    {
        if (responseRequired != 0)
            *responseRequired = false;
        if (response != 0)
            Protocol::AddCallOutcome(response, qResult(static_cast<int>(0x80010002u)));
        return;
    }
    ProtocolCallContext* context = request == 0 ? 0 : Protocol::RetrieveCallContext(request);
    if (context == 0)
    {
        if (responseRequired != 0)
            *responseRequired = false;
        if (response != 0)
            Protocol::AddCallOutcome(response, qResult(static_cast<int>(0x80010002u)));
        return;
    }
    unsigned int attemptCount = 0;
    Operation* current = GetCurrentOperation();
    if (current != 0)
    {
        CallProtocolMethodOperation* call = static_cast<CallProtocolMethodOperation*>(current);
        attemptCount = call->GetAttemptCount();
    }
    protocol->DispatchCallRequest(attemptCount, request, response, responseRequired, 0, endPoint);
    if (response != 0 && responseRequired != 0 && *responseRequired && current != 0)
    {
        Message payload;
        payload.AddDataFromMessage(*response);
        response->Clear();
        const unsigned int contextId = *reinterpret_cast<unsigned int*>(
            reinterpret_cast<unsigned char*>(context) + 0x40);
        Protocol::AddCallResponse(response, static_cast<CallProtocolMethodOperation*>(current),
                                  contextId, payload);
    }
}

void ProtocolRequestBroker::ProcessCallResponse(unsigned short protocolId, Message* response,
                                                EndPoint* endPoint)
{
    Protocol* protocol = GetProtocol(protocolId, Protocol::TYPE_CLIENT);
    ClientProtocol* client = static_cast<ClientProtocol*>(protocol);
    if (client != 0)
        client->ProcessResponse(response, endPoint);
}

void ProtocolRequestBroker::DispatchCallToProtocol(ServerProtocol* protocol, Message* request,
                                                    Message* response, bool* responseRequired,
                                                    EndPoint* endPoint)
{
    ProcessLocalCallRequest(protocol, request, response, responseRequired, endPoint);
}

bool ProtocolRequestBroker::RegisterProtocol(Protocol* protocol)
{
    BrokerState* state = StateFor(this);
    const bool registered = state->registry.RegisterProtocol(protocol) != 0;
    if (registered)
        state->protocols.push_back(protocol);
    return registered;
}

void ProtocolRequestBroker::UnregisterProtocol(Protocol* protocol)
{
    BrokerState* state = StateFor(this);
    state->registry.UnregisterProtocol(protocol);
    for (std::vector<Protocol*>::iterator it = state->protocols.begin();
         it != state->protocols.end(); ++it)
    {
        if (*it == protocol)
        {
            state->protocols.erase(it);
            break;
        }
    }
}

Protocol* ProtocolRequestBroker::GetProtocol(unsigned short protocolId, Protocol::_Type type)
{
    return StateFor(this)->registry.GetProtocol(protocolId, type);
}

bool ProtocolRequestBroker::CloneProtocols()
{
    std::vector<Protocol*> clones;
    BrokerState* state = StateFor(this);
    for (std::vector<Protocol*>::const_iterator it = state->protocols.begin();
         it != state->protocols.end(); ++it)
    {
        if (*it != 0)
        {
            Protocol* clone = (*it)->Clone();
            if (clone != 0)
                clones.push_back(clone);
        }
    }
    for (std::vector<Protocol*>::const_iterator it = clones.begin(); it != clones.end(); ++it)
        RegisterProtocol(*it);
    return true;
}

bool ProtocolRequestBroker::RegisterServerProtocol(unsigned short protocolId,
                                                    ServerProtocol* protocol)
{
    protocol->SetProtocolID(protocolId);
    RegisterProtocol(protocol);
    return true;
}

bool ProtocolRequestBroker::UnregisterServerProtocol(unsigned short protocolId)
{
    ServerProtocol* protocol = GetServerProtocol(protocolId);
    if (protocol == 0)
        return false;
    BrokerState* state = StateFor(this);
    state->registry.UnregisterProtocol(protocol);
    for (std::vector<Protocol*>::iterator it = state->protocols.begin();
         it != state->protocols.end(); ++it)
    {
        if (*it == protocol)
        {
            state->protocols.erase(it);
            break;
        }
    }
    return true;
}

ServerProtocol* ProtocolRequestBroker::GetServerProtocol(unsigned short protocolId)
{
    return static_cast<ServerProtocol*>(GetProtocol(protocolId, Protocol::TYPE_SERVER));
}

qResult ProtocolRequestBroker::DispatchLocalRMC(Message* message, ClientProtocol* client,
                                                ServerProtocol* server, EndPoint* endPoint)
{
    qResult result = server->FlagIsSet(PROTOCOL_FLAG_THREAD_SAFE_RMC)
                         ? DispatchThreadSafeLocalRMC(message, client, server, endPoint)
                         : DispatchBlockingLocalRMC(message, client, server, endPoint);
    return result;
}

qResult ProtocolRequestBroker::DispatchBlockingLocalRMC(Message* message, ClientProtocol* client,
                                                        ServerProtocol* server, EndPoint* endPoint)
{
    if (server == 0 || client == 0)
        return qResult(false);
    server->AssociateProtocolRequestBroker(this);
    CallProtocolMethodOperation operation;
    m_operationManager.OperationBegins(&operation);
    bool responseRequired = true;
    Message response;
    ProcessLocalCallRequest(server, message, &response, &responseRequired, endPoint);
    if (responseRequired)
        client->ProcessResponse(&response, endPoint);
    m_operationManager.OperationEnds(&operation);
    return qResult(true);
}

qResult ProtocolRequestBroker::DispatchThreadSafeLocalRMC(Message* message, ClientProtocol* client,
                                                          ServerProtocol* server, EndPoint* endPoint)
{
    StateFor(this)->policy.BeginCallImpl();
    if (server != 0)
        server->AssociateProtocolRequestBroker(this);
    CallProtocolMethodOperation operation;
    m_operationManager.OperationBegins(&operation);
    bool responseRequired = true;
    Message response;
    ProcessLocalCallRequest(server, message, &response, &responseRequired, endPoint);
    qResult result = qResult(true);
    if (client != 0 && responseRequired)
        client->ProcessResponse(&response, endPoint);
    m_operationManager.OperationEnds(&operation);
    StateFor(this)->policy.EndCallImpl();
    return result;
}

void ProtocolRequestBroker::PeerDisconnected(EndPoint* endPoint)
{
    SecurityContextManager* security = SecurityContextManager::GetInstance();
    if (security != 0 && endPoint != 0)
        security->Push(g_protocolCurrentPID == 0 ? 0 : g_protocolCurrentPID(),
                       g_protocolCurrentCID == 0 ? 0 : g_protocolCurrentCID(),
                       *reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(endPoint) + 0x4C));
    for (std::vector<Protocol*>::iterator it = StateFor(this)->protocols.begin();
         it != StateFor(this)->protocols.end(); ++it)
        if (*it != 0)
            (*it)->EndPointDisconnected(endPoint);
    if (Core::GetInstance() != 0 && Core::GetInstance()->GetCallContextRegister() != 0 &&
        endPoint != 0)
    {
        const unsigned int connectionId = *reinterpret_cast<unsigned int*>(
            reinterpret_cast<unsigned char*>(endPoint) + 0x48);
        Core::GetInstance()->GetCallContextRegister()->CancelCallContexts(endPoint, connectionId);
    }
    if (security != 0 && endPoint != 0)
        security->Pop();
}

void ProtocolRequestBroker::FaultDetection(EndPoint* endPoint, unsigned int reason)
{
    SecurityContextManager* security = SecurityContextManager::GetInstance();
    if (security != 0 && endPoint != 0)
        security->Push(g_protocolCurrentPID == 0 ? 0 : g_protocolCurrentPID(),
                       g_protocolCurrentCID == 0 ? 0 : g_protocolCurrentCID(),
                       *reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(endPoint) + 0x4C));
    for (std::vector<Protocol*>::iterator it = StateFor(this)->protocols.begin();
         it != StateFor(this)->protocols.end(); ++it)
        if (*it != 0)
            (*it)->FaultDetected(endPoint, reason);
    if (security != 0 && endPoint != 0)
        security->Pop();
}

}}
