#include "nn/nex/Protocol.hpp"

#include <cwchar>

#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CallProtocolMethodOperation.hpp"

namespace nn
{
namespace nex
{

Protocol::Protocol(unsigned int uiFlags)
    : SystemComponent(String(L"<unknown>"))
    , m_uiProtocolID(0)
    , m_pOutgoingConnection(0)
    , m_pProtocolRequestBroker(0)
    , m_uiFlags(uiFlags)
    , m_pIncomingConnection(0)
    , m_bLocalLoopback(false)
    , m_uiCallerPID(0)
    , m_uiCallerCID(0)
{
}

Protocol::~Protocol()
{
}

const wchar_t* Protocol::GetType() const
{
    return L"Protocol";
}

bool Protocol::IsAKindOf(const wchar_t* pType) const
{
    return std::wcscmp(L"Protocol", pType) == 0 || std::wcscmp(L"SystemComponent", pType) == 0;
}

void Protocol::EnforceDeclareSysComponentMacro()
{
}

void Protocol::EndPointDisconnected(EndPoint*)
{
}

void Protocol::FaultDetected(EndPoint*, unsigned int)
{
}

Protocol* Protocol::Clone() const
{
    return 0;
}

bool Protocol::FlagIsSet(unsigned int uiFlag) const
{
    return (m_uiFlags & uiFlag) == uiFlag;
}

void Protocol::SetFlag(unsigned int uiFlag)
{
    m_uiFlags |= uiFlag;
}

void Protocol::SetProtocolID(unsigned short uiProtocolID)
{
    m_uiProtocolID = uiProtocolID;
}

bool Protocol::BeginInitialization()
{
    if (FlagIsSet(1) && m_pProtocolRequestBroker != 0)
        m_pProtocolRequestBroker->RegisterProtocol(this);
    return true;
}

bool Protocol::BeginTermination()
{
    if (FlagIsSet(1) && m_pProtocolRequestBroker != 0)
        m_pProtocolRequestBroker->UnregisterProtocol(this);
    return true;
}

void Protocol::SetIncomingConnection(EndPoint* pEndPoint)
{
    if (m_bLocalLoopback && pEndPoint == 0)
        return;

    m_pIncomingConnection = pEndPoint;
    m_bLocalLoopback = false;
    if (pEndPoint == 0)
    {
        m_uiCallerPID = 0;
        m_uiCallerCID = 0;
        return;
    }

    m_uiCallerPID = *reinterpret_cast<const unsigned int*>(
        reinterpret_cast<const unsigned char*>(pEndPoint) + 0x4C);
    m_uiCallerCID = *reinterpret_cast<const unsigned int*>(
        reinterpret_cast<const unsigned char*>(pEndPoint) + 0x48);
}

EndPoint* Protocol::GetOutgoingConnection() const
{
    return m_pOutgoingConnection;
}

void Protocol::SetOutgoingConnection(EndPoint* pEndPoint)
{
    m_pOutgoingConnection = pEndPoint;
}

void Protocol::UseLocalLoopback(unsigned int uiCallerPID, unsigned int uiCallerCID)
{
    m_uiCallerPID = uiCallerPID;
    m_uiCallerCID = uiCallerCID;
    m_pIncomingConnection = 0;
    m_bLocalLoopback = true;
}

unsigned int Protocol::GetCallerPID() const
{
    return m_uiCallerPID;
}

unsigned int Protocol::GetCallerCID() const
{
    return m_uiCallerCID;
}

void Protocol::AssociateProtocolRequestBroker(ProtocolRequestBrokerInterface* pRequestBroker)
{
    m_pProtocolRequestBroker = pRequestBroker;
}

OperationManager* Protocol::GetOperationManager() const
{
    return &m_pProtocolRequestBroker->m_operationManager;
}

Operation* Protocol::GetCurrentOperation()
{
    return GetOperationManager()->GetCurrentOperation();
}

void Protocol::AddProtocolKey(Message* pMessage, _Command command, unsigned short uiProtocolID)
{
    if (uiProtocolID < 0x7F)
    {
        unsigned char uiKey = static_cast<unsigned char>(uiProtocolID & 0x7F);
        if (command != COMMAND_RESPONSE)
            uiKey |= 0x80;
        pMessage->Append(&uiKey, 1, 1);
        return;
    }

    unsigned char uiKey = 0x7F;
    if (command != COMMAND_RESPONSE)
        uiKey |= 0x80;
    pMessage->Append(&uiKey, 1, 1);

    unsigned short uiExtendedProtocolID = uiProtocolID;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiExtendedProtocolID), 2, 1);
}

void Protocol::AddCallOutcome(Message* pMessage, qResult result)
{
    if (!static_cast<bool>(result))
    {
        unsigned char state = 0;
        pMessage->Append(&state, 1, 1);
        pMessage->Append(reinterpret_cast<const unsigned char*>(&result), 4, 1);
    }
    else
    {
        unsigned char state = 1;
        pMessage->Append(&state, 1, 1);
    }
}

void Protocol::AddCallContextID(Message* pMessage, unsigned int uiCallContextID)
{
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiCallContextID), 4, 1);
}

void Protocol::AddCallResponse(Message* pMessage, CallProtocolMethodOperation* pOperation,
                               unsigned int uiCallContextID, const Message& rResponse)
{
    const qResult& result = *reinterpret_cast<const qResult*>(
        reinterpret_cast<const unsigned char*>(pOperation) + 0x24);
    AddCallOutcome(pMessage, result);
    AddCallContextID(pMessage, uiCallContextID);
    if (static_cast<bool>(result))
        pMessage->AddDataFromMessage(rResponse);
}

void Protocol::ExtractProtocolKey(Message* pMessage, _Command& rCommand,
                                  unsigned short& rProtocolID)
{
    unsigned char key = 0;
    pMessage->ExtractPrimitive(&key);
    rProtocolID = static_cast<unsigned short>(key & 0x7F);
    rCommand = static_cast<_Command>(key >> 7);
    if (rProtocolID == 0x7F)
        pMessage->ExtractPrimitive(&rProtocolID);
}

void Protocol::ExtractCallOutcome(Message* pMessage, qResult* pResult)
{
    unsigned char state = 0xFF;
    pMessage->ExtractPrimitive(&state);
    if (state == 0)
    {
        int code = 0;
        pMessage->ExtractPrimitive(&code);
        *pResult = qResult(code);
    }
    else if (state == 1)
        *pResult = qResult(0x10001);
    else
        *pResult = qResult(static_cast<int>(0x80010001));
}

bool Protocol::RegisterCallContext(Message* pMessage, ProtocolCallContext* pCallContext)
{
    CallContext* pContext = reinterpret_cast<CallContext*>(pCallContext);
    if (!pContext->InitiateCall())
        return false;
    AddCallContextID(pMessage, *reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(pContext) + 0x40));
    return true;
}

unsigned int Protocol::ExtractCallContextID(Message* pMessage)
{
    unsigned int id = 0;
    pMessage->ExtractPrimitive(&id);
    return id;
}

ProtocolCallContext* Protocol::RetrieveCallContext(Message* pMessage)
{
    const unsigned int id = ExtractCallContextID(pMessage);
    Core* pCore = Core::GetInstance();
    if (pCore == 0 || pCore->GetCallContextRegister() == 0)
        return 0;
    return reinterpret_cast<ProtocolCallContext*>(
        pCore->GetCallContextRegister()->GetCallContext(id));
}

bool Protocol::IsOldRVDDLVersion(EndPoint* pEndPoint)
{
    if (pEndPoint == 0)
        return false;
    const unsigned int principalId = *reinterpret_cast<const unsigned int*>(
        reinterpret_cast<const unsigned char*>(pEndPoint) + 0x4C);
    typedef unsigned long long (*GetSupportedFunctionsFlagsMethod)(const EndPoint*);
    void** pVTable = *reinterpret_cast<void***>(pEndPoint);
    const unsigned long long flags = reinterpret_cast<GetSupportedFunctionsFlagsMethod>(
        pVTable[63])(pEndPoint);
    return (principalId - 0x6E < 10) || (flags & 0x08000000u) == 0;
}

unsigned int Protocol::ExtractMethodID(Message* pMessage)
{
    unsigned int uiMethodID;
    pMessage->ExtractPrimitive(&uiMethodID);
    return uiMethodID;
}

void Protocol::AddMethodID(Message* pMessage, unsigned int uiMethodID)
{
    unsigned int uiValue = uiMethodID;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiValue), 4, 1);
}

}
}
