#ifndef SOURCE_NN_NEX_MATCHMAKINGPROTOCOLCLIENT_HPP
#define SOURCE_NN_NEX_MATCHMAKINGPROTOCOLCLIENT_HPP

#include "nn/nex/ClientProtocol.hpp"

namespace nn { namespace nex {

class MatchMakingProtocolClient : public ClientProtocol
{
public:
    explicit MatchMakingProtocolClient(unsigned short protocol_id);
    virtual ~MatchMakingProtocolClient();
};

}}

#endif
