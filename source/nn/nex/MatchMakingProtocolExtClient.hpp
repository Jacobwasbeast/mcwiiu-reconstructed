#ifndef SOURCE_NN_NEX_MATCHMAKINGPROTOCOLEXTCLIENT_HPP
#define SOURCE_NN_NEX_MATCHMAKINGPROTOCOLEXTCLIENT_HPP

#include "nn/nex/ClientProtocol.hpp"

namespace nn { namespace nex {

class MatchMakingProtocolExtClient : public ClientProtocol
{
public:
    explicit MatchMakingProtocolExtClient(unsigned short protocol_id = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);
    virtual Protocol* CreateResponder() const;
};

}}

#endif
