#include "nn/nex/Reconstruction.hpp"

namespace nn
{
namespace nex
{

MatchmakeExtensionClient::MatchmakeExtensionClient()
    : MatchMakingClient()
    , m_protocolRegistered(false)
    , m_matchmakeExtensionProtocolClient(0)
    , m_uiLastProgressScoreTime(0)
{
    m_matchmakeExtensionProtocolClient.SetProtocolID(0x6d);
}

MatchmakeExtensionClient::~MatchmakeExtensionClient()
{
    if (m_protocolRegistered)
    {
        UnregisterProtocol(&m_matchmakeExtensionProtocolClient);
    }
}

bool MatchmakeExtensionClient::Bind(Credentials* pCredentials)
{
    const bool bBound = ServiceClient::Bind(pCredentials);
    if (bBound && !m_protocolRegistered)
    {
        RegisterProtocol(&m_matchmakeExtensionProtocolClient);
        m_protocolRegistered = true;
    }
    return bBound;
}

bool MatchmakeExtensionClient::Unbind()
{
    const bool bUnbound = ServiceClient::Unbind();
    if (bUnbound && m_protocolRegistered)
    {
        UnregisterProtocol(&m_matchmakeExtensionProtocolClient);
    }
    m_protocolRegistered = false;
    return bUnbound;
}

}
}
