#include "nn/nex/MatchMakingProtocolClient.hpp"

namespace nn { namespace nex {

MatchMakingProtocolClient::MatchMakingProtocolClient(unsigned short protocol_id)
    : ClientProtocol(1)
{
    if (protocol_id != 0)
        SetProtocolID(protocol_id);
}

MatchMakingProtocolClient::~MatchMakingProtocolClient()
{
}

}}
