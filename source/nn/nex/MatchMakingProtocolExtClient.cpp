#include "nn/nex/MatchMakingProtocolExtClient.hpp"

#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"

namespace nn { namespace nex {

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);

const unsigned int METHOD_ID_MASK = 0x7FFF;
const unsigned int METHOD_ID_COUNT = 7;

ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

MatchMakingProtocolExtClient::MatchMakingProtocolExtClient(unsigned short protocol_id)
    : ClientProtocol(1)
{
    if (protocol_id != 0)
        SetProtocolID(protocol_id);
}

void MatchMakingProtocolExtClient::ExtractCallSpecificResults(Message* pMessage,
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

Protocol* MatchMakingProtocolExtClient::CreateResponder() const
{
    return new MatchMakingProtocolExtClient();
}

}}
