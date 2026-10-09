#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECLIENTTEMPLATE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECLIENTTEMPLATE_HPP

#include <stdint.h>
#include <utility>

#include "nn/nex/DataStoreClientInterface.hpp"
#include "nn/nex/DataStoreLogicServerClient.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class DataStoreClientTemplate : public T, public DataStoreClientInterface
{
public:
    DataStoreClientTemplate();
    virtual ~DataStoreClientTemplate();

    virtual bool SetDependentRvConnection(ProtocolCallContext* pCallContext) const;

    virtual bool GetObject(ProtocolCallContext* pContext,
                           const DataStorePrepareGetParam& rParam,
                           DataStoreGetObjectEventListenerTemplate<uint64_t>* pListener);
    virtual bool GetObject(ProtocolCallContext* pContext,
                           const DataStorePrepareGetParam& rParam,
                           void* pBuffer,
                           uint32_t uiBufferSize,
                           uint32_t* pReceivedSize);
    virtual bool GetObjectOrMetaBinary(ProtocolCallContext* pContext,
                                       const DataStorePrepareGetParam& rParam,
                                       DataStoreGetObjectEventListenerTemplate<uint64_t>* pListener);
    virtual bool GetObjectV1(ProtocolCallContext* pContext,
                             const DataStorePrepareGetParamV1& rParam,
                             DataStoreGetObjectEventListenerTemplate<uint32_t>* pListener);
    virtual bool PostObject(ProtocolCallContext* pContext,
                            const DataStorePreparePostParam& rParam,
                            uint64_t* pDataId,
                            DataStorePostObjectEventListener* pListener);
    virtual bool PostObject(ProtocolCallContext* pContext,
                            const DataStorePreparePostParam& rParam,
                            uint64_t* pDataId,
                            const void* pData,
                            bool bCopyData);
    virtual bool PostObject(ProtocolCallContext* pContext,
                            const DataStorePreparePostParam& rParam,
                            uint64_t* pDataId);
    virtual bool PostObject(ProtocolCallContext* pContext,
                            uint64_t uiDataId,
                            const DataStorePreparePostParam& rParam);
    virtual bool PostObject(
        ProtocolCallContext* pContext,
        const qVector<std::pair<uint64_t, DataStorePreparePostParam> >& rParams,
        bool bTransactional,
        qVector<qResult>* pResults);
    virtual bool PostObject(ProtocolCallContext* pContext,
                            const qVector<uint64_t>& rDataIds,
                            const DataStorePreparePostParam& rParam,
                            bool bTransactional,
                            qVector<qResult>* pResults);
    virtual bool PostObjectV1(ProtocolCallContext* pContext,
                              const DataStorePreparePostParamV1& rParam,
                              uint32_t* pDataId,
                              DataStorePostObjectEventListener* pListener);
    virtual bool UpdateObject(ProtocolCallContext* pContext,
                              const DataStorePrepareUpdateParam& rParam,
                              DataStorePostObjectEventListener* pListener);
    virtual bool UpdateObject(ProtocolCallContext* pContext,
                              const DataStorePrepareUpdateParam& rParam,
                              const void* pData,
                              bool bCopyData);
    virtual bool CheckNotification(ProtocolCallContext* pContext,
                                   const String& rNotificationUrl,
                                   String* pNextNotificationUrl,
                                   uint64_t uiLastDataId,
                                   uint64_t* pDataIds,
                                   uint32_t uiDataIdCapacity,
                                   uint32_t* pDataIdCount,
                                   uint32_t uiTimeout,
                                   bool* pbHasNext,
                                   bool bWait);
    virtual qResult CheckNewArrival(
        ProtocolCallContext* pContext,
        const unsigned char* pNotificationBuffer,
        DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint64_t>* pListener,
        bool* pbHasNext);
    virtual qResult ReceiveNewArrivedObjects(
        ProtocolCallContext* pContext,
        const unsigned char* pNotificationBuffer,
        qVector<DataStoreGetObjectEventListenerTemplate<uint64_t>*> getObjectListeners,
        DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint64_t>* pListener,
        bool* pbHasNext);
    virtual qResult ReceiveNewArrivedObjectsV1(
        ProtocolCallContext* pContext,
        const unsigned char* pNotificationBuffer,
        qVector<DataStoreGetObjectEventListenerTemplate<uint32_t>*> getObjectListeners,
        DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint32_t>* pListener,
        bool* pbHasNext);
    virtual bool GetRating(ProtocolCallContext* pContext,
                           const DataStoreRatingTarget& rTarget,
                           DataStoreRatingInfo* pRatingInfo,
                           uint64_t uiAccessPassword);
    virtual bool GetRating(ProtocolCallContext* pContext,
                           const qVector<uint64_t>& rDataIds,
                           qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfos,
                           qVector<qResult>* pResults,
                           uint64_t uiAccessPassword);
    virtual bool GetRating(ProtocolCallContext* pContext,
                           uint64_t uiDataId,
                           qMap<signed char, DataStoreRatingInfo>* pRatings,
                           uint64_t uiAccessPassword);
    virtual bool GetRating(ProtocolCallContext* pContext,
                           const qVector<uint64_t>& rDataIds,
                           qVector<qMap<signed char, DataStoreRatingInfo> >* pRatings,
                           qVector<qResult>* pResults);
    virtual bool GetRating(ProtocolCallContext* pContext,
                           const DataStoreRatingTarget& rTarget,
                           DataStoreRatingInfo* pRatingInfo,
                           DataStoreRatingLog* pRatingLog,
                           uint64_t uiAccessPassword);
    virtual bool GetObjectByInfo(ProtocolCallContext* pContext,
                                 const DataStoreReqGetInfo& rReqGetInfo,
                                 DataStoreGetObjectEventListenerTemplate<uint64_t>* pListener);
    virtual bool GetObjectByInfo(ProtocolCallContext* pContext,
                                 const DataStoreReqGetInfo& rReqGetInfo,
                                 void* pBuffer,
                                 uint32_t uiBufferSize,
                                 uint32_t* pReceivedSize);
};

typedef DataStoreClientTemplate<DataStoreLogicServerClient> DataStoreClient;

}
}

#endif
