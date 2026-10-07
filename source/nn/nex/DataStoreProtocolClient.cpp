#include "nn/nex/DataStoreProtocolClient.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/_DDL_DataStoreMetaInfo.hpp"
#include "nn/nex/_DDL_DataStoreRatingInfo.hpp"
#include "nn/nex/_DDL_DataStoreSearchParam.hpp"
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
enum MethodId
{
    METHOD_PREPARE_GET_OBJECT_V1 = 1,
    METHOD_PREPARE_POST_OBJECT_V1 = 2,
    METHOD_COMPLETE_POST_OBJECT_V1 = 3,
    METHOD_DELETE_OBJECT = 4,
    METHOD_DELETE_OBJECTS = 5,
    METHOD_CHANGE_META_V1 = 6,
    METHOD_CHANGE_METAS_V1 = 7,
    METHOD_GET_META = 8,
    METHOD_GET_METAS = 9,
    METHOD_PREPARE_UPDATE_OBJECT = 10,
    METHOD_COMPLETE_UPDATE_OBJECT = 11,
    METHOD_SEARCH_OBJECT = 12,
    METHOD_GET_NOTIFICATION_URL = 13,
    METHOD_GET_NEW_ARRIVED_NOTIFICATIONS_V1 = 14,
    METHOD_RATE_OBJECT = 15,
    METHOD_GET_RATING = 16,
    METHOD_GET_RATINGS = 17,
    METHOD_RESET_RATING = 18,
    METHOD_RESET_RATINGS = 19,
    METHOD_GET_SPECIFIC_META_V1 = 20,
    METHOD_POST_META_BINARY = 21,
    METHOD_TOUCH_OBJECT = 22,
    METHOD_GET_RATING_WITH_LOG = 23,
    METHOD_PREPARE_POST_OBJECT = 24,
    METHOD_PREPARE_GET_OBJECT = 25,
    METHOD_COMPLETE_POST_OBJECT = 26,
    METHOD_GET_NEW_ARRIVED_NOTIFICATIONS = 27,
    METHOD_GET_SPECIFIC_META = 28,
    METHOD_GET_PERSISTENCE_INFO = 29,
    METHOD_GET_PERSISTENCE_INFOS = 30,
    METHOD_PERPETUATE_OBJECT = 31,
    METHOD_UNPERPETUATE_OBJECT = 32,
    METHOD_PREPARE_GET_OBJECT_OR_META_BINARY = 33,
    METHOD_GET_PASSWORD_INFO = 34,
    METHOD_GET_PASSWORD_INFOS = 35,
    METHOD_GET_METAS_MULTIPLE_PARAM = 36,
    METHOD_COMPLETE_POST_OBJECTS = 37,
    METHOD_CHANGE_META = 38,
    METHOD_CHANGE_METAS = 39,
    METHOD_RATE_OBJECTS = 40,
    METHOD_POST_META_BINARY_WITH_DATA_ID = 41,
    METHOD_POST_META_BINARIES_WITH_DATA_ID = 42,
    METHOD_RATE_OBJECT_WITH_POSTING = 43,
    METHOD_RATE_OBJECTS_WITH_POSTING = 44,
    METHOD_GET_OBJECT_INFOS = 45,
    METHOD_SEARCH_OBJECT_LIGHT = 46
};
ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

DataStoreProtocolClient::DataStoreProtocolClient(unsigned short uiProtocolID)
    : ClientProtocol(1)
{
    if (uiProtocolID == 0)
        return;
    SetProtocolID(uiProtocolID);
    RVClientCore::AcquireInstance()->GetProtocolRegistry()->RegisterProtocol(
        uiProtocolID, new DataStoreProtocolClient());
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
    return new DataStoreProtocolClient();
}

bool DataStoreProtocolClient::CallCompletePostObjects(ProtocolCallContext* pCallContext,
                                                      const qVector<unsigned long long>& rDataIds)
{
    Message message;
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_COMPLETE_POST_OBJECTS);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    s_ProtoReturn[METHOD_COMPLETE_POST_OBJECTS] = ProtoReturn_CompletePostObjects;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::CompletePostObjects_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                 Message* pMessage,
                                                                 const qVector<unsigned long long>& rDataIds)
{
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_COMPLETE_POST_OBJECTS);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    s_ProtoReturn[METHOD_COMPLETE_POST_OBJECTS] = ProtoReturn_CompletePostObjects;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallGetObjectInfos(ProtocolCallContext* pCallContext,
                                                 const qVector<unsigned long long>& rDataIds,
                                                 qVector<DataStoreReqGetInfo>* pReqGetInfos,
                                                 qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_OBJECT_INFOS);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqGetInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_OBJECT_INFOS] = ProtoReturn_GetObjectInfos;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::GetObjectInfos_PrepareRequest(ProtocolCallContext* pCallContext,
                                                            Message* pMessage,
                                                            const qVector<unsigned long long>& rDataIds,
                                                            qVector<DataStoreReqGetInfo>* pReqGetInfos,
                                                            qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_OBJECT_INFOS);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqGetInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_OBJECT_INFOS] = ProtoReturn_GetObjectInfos;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallGetPasswordInfo(ProtocolCallContext* pCallContext,
                                                  const unsigned long long& rDataId,
                                                  DataStorePasswordInfo* pPasswordInfo)
{
    Message message;
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PASSWORD_INFO);
    _Type_uint64::Add(&message, rDataId);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPasswordInfo);
    }
    s_ProtoReturn[METHOD_GET_PASSWORD_INFO] = ProtoReturn_GetPasswordInfo;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::GetPasswordInfo_PrepareRequest(ProtocolCallContext* pCallContext,
                                                             Message* pMessage,
                                                             const unsigned long long& rDataId,
                                                             DataStorePasswordInfo* pPasswordInfo)
{
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_PASSWORD_INFO);
    _Type_uint64::Add(pMessage, rDataId);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPasswordInfo);
    }
    s_ProtoReturn[METHOD_GET_PASSWORD_INFO] = ProtoReturn_GetPasswordInfo;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallGetPasswordInfos(ProtocolCallContext* pCallContext,
                                                   const qVector<unsigned long long>& rDataIds,
                                                   qVector<DataStorePasswordInfo>* pPasswordInfos,
                                                   qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PASSWORD_INFOS);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPasswordInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_PASSWORD_INFOS] = ProtoReturn_GetPasswordInfos;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::GetPasswordInfos_PrepareRequest(ProtocolCallContext* pCallContext,
                                                              Message* pMessage,
                                                              const qVector<unsigned long long>& rDataIds,
                                                              qVector<DataStorePasswordInfo>* pPasswordInfos,
                                                              qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_PASSWORD_INFOS);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPasswordInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_PASSWORD_INFOS] = ProtoReturn_GetPasswordInfos;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallGetPersistenceInfo(ProtocolCallContext* pCallContext,
                                                     const unsigned int& rValue,
                                                     const unsigned short& rSlot,
                                                     DataStorePersistenceInfo* pPersistenceInfo)
{
    Message message;
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PERSISTENCE_INFO);
    _Type_uint32::Add(&message, rValue);
    _Type_uint16::Add(&message, rSlot);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPersistenceInfo);
    }
    s_ProtoReturn[METHOD_GET_PERSISTENCE_INFO] = ProtoReturn_GetPersistenceInfo;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::GetPersistenceInfo_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                Message* pMessage,
                                                                const unsigned int& rValue,
                                                                const unsigned short& rSlot,
                                                                DataStorePersistenceInfo* pPersistenceInfo)
{
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_PERSISTENCE_INFO);
    _Type_uint32::Add(pMessage, rValue);
    _Type_uint16::Add(pMessage, rSlot);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPersistenceInfo);
    }
    s_ProtoReturn[METHOD_GET_PERSISTENCE_INFO] = ProtoReturn_GetPersistenceInfo;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallGetPersistenceInfos(ProtocolCallContext* pCallContext,
                                                      const unsigned int& rValue,
                                                      const qVector<unsigned short>& rSlots,
                                                      qVector<DataStorePersistenceInfo>* pPersistenceInfos,
                                                      qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PERSISTENCE_INFOS);
    _Type_uint32::Add(&message, rValue);
    _Type_qvector<_Type_uint16>::Add(&message, rSlots);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPersistenceInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_PERSISTENCE_INFOS] = ProtoReturn_GetPersistenceInfos;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::GetPersistenceInfos_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                 Message* pMessage,
                                                                 const unsigned int& rValue,
                                                                 const qVector<unsigned short>& rSlots,
                                                                 qVector<DataStorePersistenceInfo>* pPersistenceInfos,
                                                                 qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_PERSISTENCE_INFOS);
    _Type_uint32::Add(pMessage, rValue);
    _Type_qvector<_Type_uint16>::Add(pMessage, rSlots);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPersistenceInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_PERSISTENCE_INFOS] = ProtoReturn_GetPersistenceInfos;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallGetRatings(ProtocolCallContext* pCallContext,
                                             const qVector<unsigned long long>& rDataIds,
                                             qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfoWithSlotss,
                                             qVector<qResult>* pResults,
                                             const unsigned long long& rDataId)
{
    Message message;
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_RATINGS);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    _Type_uint64::Add(&message, rDataId);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfoWithSlotss);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_RATINGS] = ProtoReturn_GetRatings;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::GetRatings_PrepareRequest(ProtocolCallContext* pCallContext,
                                                        Message* pMessage,
                                                        const qVector<unsigned long long>& rDataIds,
                                                        qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfoWithSlotss,
                                                        qVector<qResult>* pResults,
                                                        const unsigned long long& rDataId)
{
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_RATINGS);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    _Type_uint64::Add(pMessage, rDataId);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfoWithSlotss);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_RATINGS] = ProtoReturn_GetRatings;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallPerpetuateObject(ProtocolCallContext* pCallContext,
                                                   const unsigned short& rSlot,
                                                   const unsigned long long& rDataId,
                                                   const bool& rFlag)
{
    Message message;
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_PERPETUATE_OBJECT);
    _Type_uint16::Add(&message, rSlot);
    _Type_uint64::Add(&message, rDataId);
    message << rFlag;
    s_ProtoReturn[METHOD_PERPETUATE_OBJECT] = ProtoReturn_PerpetuateObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::PerpetuateObject_PrepareRequest(ProtocolCallContext* pCallContext,
                                                              Message* pMessage,
                                                              const unsigned short& rSlot,
                                                              const unsigned long long& rDataId,
                                                              const bool& rFlag)
{
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_PERPETUATE_OBJECT);
    _Type_uint16::Add(pMessage, rSlot);
    _Type_uint64::Add(pMessage, rDataId);
    *pMessage << rFlag;
    s_ProtoReturn[METHOD_PERPETUATE_OBJECT] = ProtoReturn_PerpetuateObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallResetRatings(ProtocolCallContext* pCallContext,
                                               const qVector<unsigned long long>& rDataIds,
                                               const bool& rFlag,
                                               qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_RESET_RATINGS);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    message << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_RESET_RATINGS] = ProtoReturn_ResetRatings;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::ResetRatings_PrepareRequest(ProtocolCallContext* pCallContext,
                                                          Message* pMessage,
                                                          const qVector<unsigned long long>& rDataIds,
                                                          const bool& rFlag,
                                                          qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_RESET_RATINGS);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    *pMessage << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_RESET_RATINGS] = ProtoReturn_ResetRatings;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallSearchObject(ProtocolCallContext* pCallContext,
                                               const DataStoreSearchParam& rSearchParam,
                                               DataStoreSearchResult* pSearchResult)
{
    Message message;
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_SEARCH_OBJECT);
    _DDL_DataStoreSearchParam::Add(&message, rSearchParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSearchResult);
    }
    s_ProtoReturn[METHOD_SEARCH_OBJECT] = ProtoReturn_SearchObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::SearchObject_PrepareRequest(ProtocolCallContext* pCallContext,
                                                          Message* pMessage,
                                                          const DataStoreSearchParam& rSearchParam,
                                                          DataStoreSearchResult* pSearchResult)
{
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_SEARCH_OBJECT);
    _DDL_DataStoreSearchParam::Add(pMessage, rSearchParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSearchResult);
    }
    s_ProtoReturn[METHOD_SEARCH_OBJECT] = ProtoReturn_SearchObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallSearchObjectLight(ProtocolCallContext* pCallContext,
                                                    const DataStoreSearchParam& rSearchParam,
                                                    DataStoreSearchResult* pSearchResult)
{
    Message message;
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_SEARCH_OBJECT_LIGHT);
    _DDL_DataStoreSearchParam::Add(&message, rSearchParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSearchResult);
    }
    s_ProtoReturn[METHOD_SEARCH_OBJECT_LIGHT] = ProtoReturn_SearchObjectLight;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::SearchObjectLight_PrepareRequest(ProtocolCallContext* pCallContext,
                                                               Message* pMessage,
                                                               const DataStoreSearchParam& rSearchParam,
                                                               DataStoreSearchResult* pSearchResult)
{
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_SEARCH_OBJECT_LIGHT);
    _DDL_DataStoreSearchParam::Add(pMessage, rSearchParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSearchResult);
    }
    s_ProtoReturn[METHOD_SEARCH_OBJECT_LIGHT] = ProtoReturn_SearchObjectLight;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallUnperpetuateObject(ProtocolCallContext* pCallContext,
                                                     const unsigned short& rSlot,
                                                     const bool& rFlag)
{
    Message message;
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UNPERPETUATE_OBJECT);
    _Type_uint16::Add(&message, rSlot);
    message << rFlag;
    s_ProtoReturn[METHOD_UNPERPETUATE_OBJECT] = ProtoReturn_UnperpetuateObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::UnperpetuateObject_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                Message* pMessage,
                                                                const unsigned short& rSlot,
                                                                const bool& rFlag)
{
    Scheduler* pScheduler = Core::GetInstance()->GetScheduler();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_UNPERPETUATE_OBJECT);
    _Type_uint16::Add(pMessage, rSlot);
    *pMessage << rFlag;
    s_ProtoReturn[METHOD_UNPERPETUATE_OBJECT] = ProtoReturn_UnperpetuateObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
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
    DataStoreSearchResult discarded[1];
    _DDL_DataStoreSearchResult::Extract(pMessage, discarded);
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
    DataStoreSearchResult discarded[1];
    _DDL_DataStoreSearchResult::Extract(pMessage, discarded);
}

void DataStoreProtocolClient::ProtoReturn_GetMeta(Message* pMessage,
                                                  ProtocolCallContext* pCallContext)
{
    DataStoreMetaInfo* pMetaInfo =
        static_cast<DataStoreMetaInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pMetaInfo != 0)
    {
        _DDL_DataStoreMetaInfo::Extract(pMessage, pMetaInfo);
        return;
    }
    DataStoreMetaInfo discarded[1];
    _DDL_DataStoreMetaInfo::Extract(pMessage, discarded);
}

void DataStoreProtocolClient::ProtoReturn_GetRating(Message* pMessage,
                                                    ProtocolCallContext* pCallContext)
{
    DataStoreRatingInfo* pRatingInfo =
        static_cast<DataStoreRatingInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pRatingInfo != 0)
    {
        _DDL_DataStoreRatingInfo::Extract(pMessage, pRatingInfo);
        return;
    }
    DataStoreRatingInfo discarded[1];
    _DDL_DataStoreRatingInfo::Extract(pMessage, discarded);
}

void DataStoreProtocolClient::ProtoReturn_RateObject(Message* pMessage,
                                                     ProtocolCallContext* pCallContext)
{
    DataStoreRatingInfo* pRatingInfo =
        static_cast<DataStoreRatingInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pRatingInfo != 0)
    {
        _DDL_DataStoreRatingInfo::Extract(pMessage, pRatingInfo);
        return;
    }
    DataStoreRatingInfo discarded[1];
    _DDL_DataStoreRatingInfo::Extract(pMessage, discarded);
}

void DataStoreProtocolClient::ProtoReturn_RateObjectWithPosting(Message* pMessage,
                                                                ProtocolCallContext* pCallContext)
{
    DataStoreRatingInfo* pRatingInfo =
        static_cast<DataStoreRatingInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pRatingInfo != 0)
    {
        _DDL_DataStoreRatingInfo::Extract(pMessage, pRatingInfo);
        return;
    }
    DataStoreRatingInfo discarded[1];
    _DDL_DataStoreRatingInfo::Extract(pMessage, discarded);
}

void DataStoreProtocolClient::ProtoReturn_PostMetaBinary(Message* pMessage,
                                                         ProtocolCallContext* pCallContext)
{
    unsigned long long discarded;
    unsigned long long* pDataId =
        static_cast<unsigned long long*>(pCallContext->GetReturnValuePtr(0));
    if (pDataId == 0)
        pDataId = &discarded;
    pMessage->ExtractPrimitive(pDataId);
}

void DataStoreProtocolClient::ProtoReturn_CompletePostObject(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_CompletePostObjectV1(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_CompletePostObjects(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_CompleteUpdateObject(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_DeleteObject(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_ChangeMeta(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_ChangeMetaV1(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_ResetRating(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_PostMetaBinaryWithDataId(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_TouchObject(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_PerpetuateObject(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_UnperpetuateObject(Message*, ProtocolCallContext*)
{
}

}
}
