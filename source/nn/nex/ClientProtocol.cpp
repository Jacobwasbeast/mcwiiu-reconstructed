#include "nn/nex/ClientProtocol.hpp"

#include <cwchar>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/ProtocolCallContext.hpp"

namespace nn
{
namespace nex
{

ClientProtocol::ClientProtocol(unsigned int uiFlags)
    : Protocol(uiFlags)
    , m_pDefaultCredentials(0)
{
}

ClientProtocol::~ClientProtocol()
{
    SetDefaultCredentials(0);
}

void ClientProtocol::SetDefaultCredentials(Credentials* pCredentials)
{
    if (m_pDefaultCredentials != 0)
    {
        m_pDefaultCredentials->ReleaseRef();
        m_pDefaultCredentials = 0;
    }

    if (pCredentials == 0)
        return;

    m_pDefaultCredentials = pCredentials;
    m_pDefaultCredentials->AddRef();
}

void ClientProtocol::SetCallError(qResult)
{
    Platform::Breakpoint();
}

void ClientProtocol::ProcessResponse(Message* pMessage, EndPoint* pEndPoint)
{
    SetIncomingConnection(pEndPoint);
    if (IsOldRVDDLVersion(pEndPoint))
        pMessage->m_bCompactFormat = 1;

    qResult result;
    ExtractCallOutcome(pMessage, &result);
    ProtocolCallContext* pCallContext = RetrieveCallContext(pMessage);
    if (pCallContext == 0)
        return;

    if (pMessage->HasError())
    {
        pCallContext->SignalFailure(qResult(static_cast<int>(0x8001000F)));
        return;
    }

    if (!static_cast<bool>(result))
    {
        pCallContext->SignalFailure(result);
        return;
    }

    ExtractCallSpecificResults(pMessage, pCallContext);
    if (pMessage->HasError())
    {
        pCallContext->SignalFailure(qResult(static_cast<int>(0x8001000F)));
        return;
    }

    pCallContext->SetStateImpl(CallContext::CallSucceeded, qResult(0x10001), true);
}

}
}
