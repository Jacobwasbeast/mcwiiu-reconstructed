#include "nn/nex/ProviderSupport.hpp"
#include "nn/nex/Reconstruction.hpp"

#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"

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

void MatchMakingClient::UpdateProtocolsDefaultCredentials(Credentials* pCredentials)
{
    m_matchMakingProtocolClient.SetDefaultCredentials(pCredentials);
    m_matchMakingProtocolExtClient.SetDefaultCredentials(pCredentials);
}

}
}
