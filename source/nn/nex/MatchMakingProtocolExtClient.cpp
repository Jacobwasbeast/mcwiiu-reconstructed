#include "nn/nex/MatchMakingProtocolExtClient.hpp"

namespace nn { namespace nex {

MatchMakingProtocolExtClient::MatchMakingProtocolExtClient(unsigned short protocol_id)
    : ClientProtocol(1)
{
    if (protocol_id != 0)
        SetProtocolID(protocol_id);
}

MatchMakingProtocolExtClient::~MatchMakingProtocolExtClient()
{
}

}}
