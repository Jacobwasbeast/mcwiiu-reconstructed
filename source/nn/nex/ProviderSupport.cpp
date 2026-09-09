#include "nn/nex/ProviderSupport.hpp"
#include "nn/nex/Reconstruction.hpp"

#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);

const unsigned int METHOD_ID_MASK = 0x7FFF;
const unsigned int METHOD_ID_COUNT = 48;

ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

MatchMakingClient::MatchMakingClient()
    : ServiceClient(0x0c)
    , m_matchMakingProtocolClient(0)
    , m_matchMakingProtocolExtClient(0)
{
    m_matchMakingProtocolClient.SetProtocolID(0x15);
    m_matchMakingProtocolExtClient.SetProtocolID(0x32);
}

MatchMakingClient::~MatchMakingClient()
{
}

void MatchMakingClient::UpdateProtocolsDefaultCredentials(Credentials* pCredentials)
{
    m_matchMakingProtocolClient.SetDefaultCredentials(pCredentials);
    m_matchMakingProtocolExtClient.SetDefaultCredentials(pCredentials);
}

MatchmakeExtensionProtocolClient::MatchmakeExtensionProtocolClient(unsigned short protocolId)
    : ClientProtocol(1)
{
    if (protocolId != 0)
    {
        SetProtocolID(protocolId);
    }
}

MatchmakeExtensionProtocolClient::~MatchmakeExtensionProtocolClient()
{
}

void MatchmakeExtensionProtocolClient::ExtractCallSpecificResults(Message* pMessage,
                                                                  ProtocolCallContext* pCallContext)
{
    const unsigned int uiMethodID = Protocol::ExtractMethodID(pMessage) & METHOD_ID_MASK;
    if (uiMethodID - 1 >= METHOD_ID_COUNT - 1)
    {
        const int iCallError = static_cast<int>(0x80010002u);
        ClientProtocol::SetCallError(qResult(iCallError));
        return;
    }

    if (s_ProtoReturn[uiMethodID] != 0)
        s_ProtoReturn[uiMethodID](pMessage, pCallContext);
}

Protocol* MatchmakeExtensionProtocolClient::CreateResponder() const
{
    return new MatchmakeExtensionProtocolClient(0);
}

}
}
