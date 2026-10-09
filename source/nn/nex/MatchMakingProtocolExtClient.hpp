#ifndef SOURCE_NN_NEX_MATCHMAKINGPROTOCOLEXTCLIENT_HPP
#define SOURCE_NN_NEX_MATCHMAKINGPROTOCOLEXTCLIENT_HPP

#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/String.hpp"

namespace nn { namespace nex {

class MatchMakingProtocolExtClient : public ClientProtocol
{
public:
    explicit MatchMakingProtocolExtClient(unsigned short protocol_id = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);
    virtual Protocol* CreateResponder() const;

    bool CallEndParticipation(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID, const String& rMessage);
    static bool EndParticipation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess, const unsigned int& rGatheringID, const String& rMessage);
    static void ProtoReturn_EndParticipation(Message* pMessage, ProtocolCallContext* pCallContext);
};

}}

#endif
