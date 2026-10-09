#include "nn/nex/MatchMakingProtocolExtClient.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn { namespace nex {

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);

const unsigned int METHOD_ID_MASK = 0x7FFF;
const unsigned int METHOD_COUNT = 6;
const unsigned int METHOD_END_PARTICIPATION = 1;
const unsigned int PROTOCOL_FLAG_UNLOCK_BEFORE_SEND = 0x10;

ProtoReturnFunction s_ProtoReturn[METHOD_COUNT];

}

MatchMakingProtocolExtClient::MatchMakingProtocolExtClient(unsigned short protocol_id)
    : ClientProtocol(1)
{
    if (protocol_id != 0)
        SetProtocolID(protocol_id);
}

void MatchMakingProtocolExtClient::ExtractCallSpecificResults(Message* pMessage,
                                                              ProtocolCallContext* pCallContext)
{
    const unsigned int uiMethodID = Protocol::ExtractMethodID(pMessage) & METHOD_ID_MASK;
    if (uiMethodID - 1 >= METHOD_COUNT)
    {
        const int iCallError = static_cast<int>(0x80010002u);
        ClientProtocol::SetCallError(qResult(iCallError));
        return;
    }
    if (s_ProtoReturn[uiMethodID - 1] != 0)
        s_ProtoReturn[uiMethodID - 1](pMessage, pCallContext);
}

Protocol* MatchMakingProtocolExtClient::CreateResponder() const
{
    return new MatchMakingProtocolExtClient();
}

bool MatchMakingProtocolExtClient::CallEndParticipation(ProtocolCallContext* pCallContext,
                                                        bool* pSuccess,
                                                        const unsigned int& rGatheringID,
                                                        const String& rMessage)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_END_PARTICIPATION);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_END_PARTICIPATION - 1] = ProtoReturn_EndParticipation;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolExtClient::EndParticipation_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                   Message* pMessage,
                                                                   bool* pSuccess,
                                                                   const unsigned int& rGatheringID,
                                                                   const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_END_PARTICIPATION);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_END_PARTICIPATION - 1] = ProtoReturn_EndParticipation;
    return true;
}

void MatchMakingProtocolExtClient::ProtoReturn_EndParticipation(Message* pMessage,
                                                                ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

}}
