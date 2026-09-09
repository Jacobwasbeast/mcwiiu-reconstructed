#include "nn/nex/DataStoreProtocolClient.hpp"

#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/_DDL_DataStoreSearchResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);

const unsigned int METHOD_ID_MASK = 0x7FFF;
const unsigned int METHOD_ID_COUNT = 47;

ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

DataStoreProtocolClient::~DataStoreProtocolClient()
{
}

void DataStoreProtocolClient::ExtractCallSpecificResults(Message* pMessage,
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

Protocol* DataStoreProtocolClient::CreateResponder() const
{
    return new DataStoreProtocolClient(0);
}

void DataStoreProtocolClient::ProtoReturn_SearchObject(Message* pMessage,
                                                       ProtocolCallContext* pCallContext)
{
    DataStoreSearchResult* pSearchResult =
        static_cast<DataStoreSearchResult*>(pCallContext->GetReturnValuePtr(0));
    if (pSearchResult != 0)
    {
        _DDL_DataStoreSearchResult::Extract(pMessage, pSearchResult);
        return;
    }

    DataStoreSearchResult discarded;
    _DDL_DataStoreSearchResult::Extract(pMessage, &discarded);
}

void DataStoreProtocolClient::ProtoReturn_SearchObjectLight(Message* pMessage,
                                                            ProtocolCallContext* pCallContext)
{
    DataStoreSearchResult* pSearchResult =
        static_cast<DataStoreSearchResult*>(pCallContext->GetReturnValuePtr(0));
    if (pSearchResult != 0)
    {
        _DDL_DataStoreSearchResult::Extract(pMessage, pSearchResult);
        return;
    }

    DataStoreSearchResult discarded;
    _DDL_DataStoreSearchResult::Extract(pMessage, &discarded);
}

}
}
