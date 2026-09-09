#include "nn/nex/ProviderSupport.hpp"
#include "nn/nex/Reconstruction.hpp"

namespace nn
{
namespace nex
{

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

}
}
