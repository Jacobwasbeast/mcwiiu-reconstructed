#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECLIENTINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECLIENTINTERFACE_HPP

#include <cstdint>
#include <utility>

#include "nn/nex/DataStoreMetaInfo.hpp"
#include "nn/nex/DataStoreRatingInfo.hpp"
#include "nn/nex/DataStoreRatingInfoWithSlot.hpp"
#include "nn/nex/DataStoreSearchParam.hpp"
#include "nn/nex/DataStoreSearchResult.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class ProtocolCallContext;
class DataStorePrepareGetParam;
class DataStorePrepareGetParamV1;
class DataStoreReqGetInfo;
class DataStoreReqGetInfoV1;
class DataStoreReqGetAdditionalMeta;
class DataStorePreparePostParam;
class DataStorePreparePostParamV1;
class DataStoreReqPostInfo;
class DataStoreReqPostInfoV1;
class DataStoreCompletePostParam;
class DataStoreCompletePostParamV1;
class DataStorePrepareUpdateParam;
class DataStoreReqUpdateInfo;
class DataStoreCompleteUpdateParam;
class DataStoreDeleteParam;
class DataStoreChangeMetaParam;
class DataStoreGetMetaParam;
class DataStoreGetNotificationUrlParam;
class DataStoreReqGetNotificationUrlInfo;
class DataStoreGetNewArrivedNotificationsParam;
class DataStoreNotification;
class DataStoreNotificationV1;
class DataStoreRatingTarget;
class DataStoreRateObjectParam;
class DataStoreGetSpecificMetaParam;
class DataStoreGetSpecificMetaParamV1;
class DataStoreSpecificMetaInfo;
class DataStoreSpecificMetaInfoV1;
class DataStoreTouchObjectParam;
class DataStoreRatingLog;
class DataStorePersistenceInfo;
class DataStorePasswordInfo;

class DataStoreClientInterface
{
public:
    virtual ~DataStoreClientInterface();

    virtual void PrepareGetObject(ProtocolCallContext* pContext,
                                  const DataStorePrepareGetParam& rParam,
                                  DataStoreReqGetInfo* pReqGetInfo) = 0;
    virtual void PrepareGetObject(ProtocolCallContext* pContext,
                                  const DataStorePrepareGetParamV1& rParam,
                                  DataStoreReqGetInfoV1* pReqGetInfo) = 0;
    virtual void PrepareGetObjectOrMetaBinary(ProtocolCallContext* pContext,
                                              const DataStorePrepareGetParam& rParam,
                                              DataStoreReqGetInfo* pReqGetInfo,
                                              DataStoreReqGetAdditionalMeta* pAdditionalMeta) = 0;
    virtual void PrepareGetObjectOrMetaBinary(ProtocolCallContext* pContext,
                                              const DataStorePrepareGetParamV1& rParam,
                                              DataStoreReqGetInfoV1* pReqGetInfo,
                                              DataStoreReqGetAdditionalMeta* pAdditionalMeta) = 0;
    virtual void PreparePostObject(ProtocolCallContext* pContext,
                                   const DataStorePreparePostParam& rParam,
                                   DataStoreReqPostInfo* pReqPostInfo) = 0;
    virtual void PreparePostObject(ProtocolCallContext* pContext,
                                   const DataStorePreparePostParamV1& rParam,
                                   DataStoreReqPostInfoV1* pReqPostInfo) = 0;
    virtual void CompletePostObject(ProtocolCallContext* pContext,
                                    const DataStoreCompletePostParam& rParam) = 0;
    virtual void CompletePostObject(ProtocolCallContext* pContext,
                                    const DataStoreCompletePostParamV1& rParam) = 0;
    virtual void CompleteSuspendedPostObject(ProtocolCallContext* pContext,
                                             const qVector<std::uint64_t>& rDataIds) = 0;
    virtual void PrepareUpdateObject(ProtocolCallContext* pContext,
                                     const DataStorePrepareUpdateParam& rParam,
                                     DataStoreReqUpdateInfo* pReqUpdateInfo) = 0;
    virtual void CompleteUpdateObject(ProtocolCallContext* pContext,
                                      const DataStoreCompleteUpdateParam& rParam) = 0;
    virtual void DeleteObject(ProtocolCallContext* pContext,
                              const DataStoreDeleteParam& rParam) = 0;
    virtual void ChangeMeta(ProtocolCallContext* pContext,
                            const DataStoreChangeMetaParam& rParam) = 0;
    virtual void GetMeta(ProtocolCallContext* pContext,
                         const DataStoreGetMetaParam& rParam,
                         DataStoreMetaInfo* pMetaInfo) = 0;
    virtual void GetMeta(ProtocolCallContext* pContext,
                         const qVector<std::uint64_t>& rDataIds,
                         const DataStoreGetMetaParam& rParam,
                         qVector<DataStoreMetaInfo>* pMetaInfos,
                         qVector<qResult>* pResults) = 0;
    virtual void GetMeta(ProtocolCallContext* pContext,
                         const qVector<DataStoreGetMetaParam>& rParams,
                         qVector<DataStoreMetaInfo>* pMetaInfos,
                         qVector<qResult>* pResults) = 0;
    virtual void SearchObject(ProtocolCallContext* pContext,
                              const DataStoreSearchParam& rParam,
                              DataStoreSearchResult* pSearchResult) = 0;
    virtual void SearchObjectLight(ProtocolCallContext* pContext,
                                   const DataStoreSearchParam& rParam,
                                   DataStoreSearchResult* pSearchResult) = 0;
    virtual void GetNotificationUrl(ProtocolCallContext* pContext,
                                    const DataStoreGetNotificationUrlParam& rParam,
                                    DataStoreReqGetNotificationUrlInfo* pInfo) = 0;
    virtual void GetNewArrivedNotifications(
        ProtocolCallContext* pContext,
        const DataStoreGetNewArrivedNotificationsParam& rParam,
        qVector<DataStoreNotification>* pNotifications,
        bool* pbHasNext) = 0;
    virtual void GetNewArrivedNotifications(
        ProtocolCallContext* pContext,
        const DataStoreGetNewArrivedNotificationsParam& rParam,
        qVector<DataStoreNotificationV1>* pNotifications,
        bool* pbHasNext) = 0;
    virtual void RateObject(ProtocolCallContext* pContext,
                            const DataStoreRatingTarget& rTarget,
                            const DataStoreRateObjectParam& rParam,
                            DataStoreRatingInfo* pRatingInfo) = 0;
    virtual void GetRating(ProtocolCallContext* pContext,
                           const DataStoreRatingTarget& rTarget,
                           DataStoreRatingInfo* pRatingInfo,
                           std::uint64_t uiAccessPassword) = 0;
    virtual void GetRating(ProtocolCallContext* pContext,
                           const qVector<std::uint64_t>& rDataIds,
                           qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfos,
                           qVector<qResult>* pResults,
                           std::uint64_t uiAccessPassword) = 0;
    virtual void ResetRating(ProtocolCallContext* pContext,
                             const DataStoreRatingTarget& rTarget,
                             std::uint64_t uiUpdatePassword) = 0;
    virtual void ResetRating(ProtocolCallContext* pContext,
                             const std::uint64_t& rDataId) = 0;
    virtual void ResetRating(ProtocolCallContext* pContext,
                             const qVector<std::uint64_t>& rDataIds,
                             bool bTransactional,
                             qVector<qResult>* pResults) = 0;
    virtual void GetSpecificMeta(ProtocolCallContext* pContext,
                                 const DataStoreGetSpecificMetaParam& rParam,
                                 qVector<DataStoreSpecificMetaInfo>* pMetaInfos) = 0;
    virtual void GetSpecificMeta(ProtocolCallContext* pContext,
                                 const DataStoreGetSpecificMetaParamV1& rParam,
                                 qVector<DataStoreSpecificMetaInfoV1>* pMetaInfos) = 0;
    virtual void PostMetaBinary(ProtocolCallContext* pContext,
                                const DataStorePreparePostParam& rParam,
                                std::uint64_t* pDataId) = 0;
    virtual void PostMetaBinaryWithDataId(ProtocolCallContext* pContext,
                                          std::uint64_t uiDataId,
                                          const DataStorePreparePostParam& rParam) = 0;
    virtual void PostMetaBinaryWithDataIds(ProtocolCallContext* pContext,
                                           const qVector<std::uint64_t>& rDataIds,
                                           const DataStorePreparePostParam& rParam,
                                           bool bTransactional,
                                           qVector<qResult>* pResults) = 0;
    virtual void PostMetaBinariesWithDataId(
        ProtocolCallContext* pContext,
        const qVector<std::pair<std::uint64_t, DataStorePreparePostParam> >& rParams,
        bool bTransactional,
        qVector<qResult>* pResults) = 0;
    virtual void TouchObject(ProtocolCallContext* pContext,
                             const DataStoreTouchObjectParam& rParam) = 0;
    virtual void GetRatingWithLog(ProtocolCallContext* pContext,
                                  const DataStoreRatingTarget& rTarget,
                                  DataStoreRatingInfo* pRatingInfo,
                                  DataStoreRatingLog* pRatingLog,
                                  std::uint64_t uiAccessPassword) = 0;
    virtual void GetPersistenceInfo(ProtocolCallContext* pContext,
                                    std::uint32_t uiOwnerId,
                                    std::uint16_t uiPersistenceSlotId,
                                    DataStorePersistenceInfo* pInfo) = 0;
    virtual void GetPersistenceInfo(ProtocolCallContext* pContext,
                                    std::uint32_t uiOwnerId,
                                    qVector<std::uint16_t> persistenceSlotIds,
                                    qVector<DataStorePersistenceInfo>* pInfos,
                                    qVector<qResult>* pResults) = 0;
    virtual void PerpetuateObject(ProtocolCallContext* pContext,
                                  std::uint16_t uiPersistenceSlotId,
                                  std::uint64_t uiDataId,
                                  bool bDeleteLastObject) = 0;
    virtual void UnperpetuateObject(ProtocolCallContext* pContext,
                                    std::uint16_t uiPersistenceSlotId,
                                    bool bDeleteLastObject) = 0;
    virtual void GetPasswordInfo(ProtocolCallContext* pContext,
                                 std::uint64_t uiDataId,
                                 DataStorePasswordInfo* pPasswordInfo) = 0;
    virtual void GetPasswordInfo(ProtocolCallContext* pContext,
                                 const qVector<std::uint64_t>& rDataIds,
                                 qVector<DataStorePasswordInfo>* pPasswordInfos,
                                 qVector<qResult>* pResults) = 0;
    virtual void GetObjectInfo(ProtocolCallContext* pContext,
                               const qVector<std::uint64_t>& rDataIds,
                               qVector<DataStoreReqGetInfo>* pReqGetInfos,
                               qVector<qResult>* pResults) = 0;
    virtual std::uint32_t GetMyPrincipalId() const = 0;
    virtual bool IsCredentialsNeeded() const = 0;
};

}
}

#endif
