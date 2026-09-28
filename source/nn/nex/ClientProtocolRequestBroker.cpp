#include "nn/nex/ClientProtocolRequestBroker.hpp"
#include "nn/nex/ProtocolRegistry.hpp"

#include "nn/nex/CallProtocolMethodOperation.hpp"
#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/Protocol.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/SecurityContextManager.hpp"

#include <map>

namespace nn { namespace nex {

namespace
{
typedef std::map<const ClientProtocolRequestBroker*, ProtocolRegistry*> BrokerRegistryMap;
BrokerRegistryMap g_brokerRegistries;

ProtocolRegistry* RegistryFor(const ClientProtocolRequestBroker* broker)
{
    BrokerRegistryMap::iterator it = g_brokerRegistries.find(broker);
    return it == g_brokerRegistries.end() ? 0 : it->second;
}
}

ClientProtocolRequestBroker::ClientProtocolRequestBroker()
    : ProtocolRequestBrokerInterface(), m_serverTree(0), m_field48(0), m_field4c(0),
      m_currentEndPoint(0),
      m_callPolicy()
{
    mVTable = 0x104B41E4u;
    g_brokerRegistries[this] = new ProtocolRegistry();
    m_callPolicy.SetMaximumRecursionLevel(3);
}

ClientProtocolRequestBroker::~ClientProtocolRequestBroker()
{
    BrokerRegistryMap::iterator it = g_brokerRegistries.find(this);
    if (it != g_brokerRegistries.end())
    {
        delete it->second;
        g_brokerRegistries.erase(it);
    }
}

Operation* ClientProtocolRequestBroker::GetCurrentOperation() const
{
    return m_operationManager.GetCurrentOperation();
}

void ClientProtocolRequestBroker::ProcessMessage(CallProtocolMethodOperation* operation,
                                                  EndPoint* endPoint, Buffer* buffer)
{
    m_currentEndPoint = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(endPoint));
    ProcessMessageCore(operation, endPoint, buffer);
    m_currentEndPoint = 0;
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
    ServerProtocol* server = GetServerProtocol(protocolId);
    ProtocolCallContext* context = request == 0 ? 0 : Protocol::RetrieveCallContext(request);
    if (server == 0 || context == 0)
    {
        if (responseRequired != 0)
            *responseRequired = false;
        if (response != 0)
            Protocol::AddCallOutcome(response, qResult(static_cast<int>(0x80010002u)));
        return;
    }
    DispatchCallToProtocol(server, request, response, responseRequired, endPoint);
    if (response != 0 && responseRequired != 0 && *responseRequired)
    {
        Operation* operation = m_operationManager.GetCurrentOperation();
        if (operation != 0)
        {
            CallProtocolMethodOperation* call = static_cast<CallProtocolMethodOperation*>(operation);
            const unsigned int contextId = *reinterpret_cast<unsigned int*>(
                reinterpret_cast<unsigned char*>(context) + 0x40);
            Message payload;
            payload.AddDataFromMessage(*response);
            response->Clear();
            Protocol::AddCallResponse(response, call, contextId, payload);
        }
    }
}

void ClientProtocolRequestBroker::ProcessCallResponse(unsigned short protocolId, Message* response,
                                                       EndPoint* endPoint)
{
    ProtocolRegistry* registry = RegistryFor(this);
    ClientProtocol* client = registry == 0 ? 0 : static_cast<ClientProtocol*>(registry->GetProtocol(
        protocolId, Protocol::TYPE_CLIENT));
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
    if (protocol == 0)
        return false;
    ProtocolRegistry* registry = RegistryFor(this);
    if (registry == 0)
        return false;
    registry->RegisterProtocol(protocolId, protocol);
    return true;
}

bool ClientProtocolRequestBroker::UnregisterServerProtocol(unsigned short protocolId)
{
    ServerProtocol* protocol = GetServerProtocol(protocolId);
    if (protocol == 0)
        return false;
    ProtocolRegistry* registry = RegistryFor(this);
    if (registry != 0)
        registry->UnregisterProtocol(protocol);
    return true;
}

ServerProtocol* ClientProtocolRequestBroker::GetServerProtocol(unsigned short protocolId)
{
    ProtocolRegistry* registry = RegistryFor(this);
    return registry == 0 ? 0 : static_cast<ServerProtocol*>(registry->GetServerProtocol(protocolId));
}

qResult ClientProtocolRequestBroker::DispatchLocalRMC(Message* message, ClientProtocol* client,
                                                       ServerProtocol* server, EndPoint* endPoint)
{
    m_callPolicy.BeginCallImpl();
    Message response;
    bool responseRequired = true;
    if (server == 0 || client == 0)
    {
        m_callPolicy.EndCallImpl();
        return qResult(static_cast<int>(0x80010002u));
    }
    CallProtocolMethodOperation operation;
    m_operationManager.OperationBegins(&operation);
    DispatchCallToProtocol(server, message, &response, &responseRequired, endPoint);
    if (responseRequired)
        client->ProcessResponse(&response, endPoint);
    m_operationManager.OperationEnds(&operation);
    m_callPolicy.EndCallImpl();
    return qResult(static_cast<int>(0x10001));
}

qResult ClientProtocolRequestBroker::DispatchBlockingLocalRMC(Message* message,
                                                               ClientProtocol* client,
                                                               ServerProtocol* server,
                                                               EndPoint* endPoint)
{
    return DispatchLocalRMC(message, client, server, endPoint);
}

qResult ClientProtocolRequestBroker::DispatchThreadSafeLocalRMC(Message* message,
                                                                 ClientProtocol* client,
                                                                 ServerProtocol* server,
                                                                 EndPoint* endPoint)
{
    return DispatchLocalRMC(message, client, server, endPoint);
}

void ClientProtocolRequestBroker::PeerDisconnected(EndPoint* endPoint)
{
    SecurityContextManager* security = SecurityContextManager::GetInstance();
    if (security != 0 && endPoint != 0)
        security->Push(g_protocolCurrentPID == 0 ? 0 : g_protocolCurrentPID(),
                       g_protocolCurrentCID == 0 ? 0 : g_protocolCurrentCID(),
                       *reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(endPoint) + 0x4C));
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

void ClientProtocolRequestBroker::FaultDetection(EndPoint* endPoint, unsigned int reason)
{
    (void)reason;
    SecurityContextManager* security = SecurityContextManager::GetInstance();
    if (security != 0 && endPoint != 0)
        security->Push(g_protocolCurrentPID == 0 ? 0 : g_protocolCurrentPID(),
                       g_protocolCurrentCID == 0 ? 0 : g_protocolCurrentCID(),
                       *reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(endPoint) + 0x4C));
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

void ClientProtocolRequestBroker::DispatchCallToProtocol(ServerProtocol* protocol, Message* request,
                                                          Message* response, bool* responseRequired,
                                                          EndPoint* endPoint)
{
    if (protocol != 0)
    {
        unsigned int attemptCount = 0;
        Operation* current = m_operationManager.GetCurrentOperation();
        if (current != 0)
            attemptCount = static_cast<CallProtocolMethodOperation*>(current)->GetAttemptCount();
        protocol->DispatchCallRequest(attemptCount, request, response, responseRequired, 0,
                                      endPoint);
    }
}


}}
