#include "nn/pia/session/JointSessionJob.hpp"

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "platform/wiiu/pead.hpp"

#include <string.h>

namespace nn { namespace pia { namespace session {

JointSessionJob::JointSessionJob()
    : common::StepSequenceJob(), m_callerContext(0), m_callContext(),
      m_processType(0), m_field5c(0), m_companionStationIdList(),
      m_companionStationIdFreeList(), m_stationIdNodeOffset(0),
      m_stationIdNodes(0), m_stationIdNodeNum(0), m_field98(0), m_field9c(0),
      m_field9d(0), m_leaveStationIdAccepted(false), m_fielda0(0),
      m_fielda2(0), m_fielda3(0), m_fielda4(0), m_fieldb0(0), m_fieldb4(0),
      m_fieldb8(0), m_fieldbc(0), m_fieldc0(0), m_fieldc4(0)
{
    JointSessionJob::InitializeImpl();
}

JointSessionJob::~JointSessionJob()
{
    JointSessionJob::FinalizeImpl();
}

void JointSessionJob::Trace(unsigned long long value) const
{
}

void JointSessionJob::InitializeImpl()
{
    const unsigned int stationIdNodeNum = transport::g_piaTransport->m_stationNum;
    const unsigned int storageSize = stationIdNodeNum * sizeof(StationIdNode);
    pead::Heap* heap = common::HeapManager::GetHeap();
    StationIdNode* storage =
        static_cast<StationIdNode*>(pead::allocateFromSeadHeap(storageSize, heap));
    if (storage == 0)
    {
        m_stationIdNodeStorage = 0;
    }
    else
    {
        memset(storage, 0, storageSize);
        m_stationIdNodeStorage = storage;
    }

    if (stationIdNodeNum == 0 || storage == 0)
        return;

    m_stationIdNodes = storage;
    m_stationIdNodeNum = stationIdNodeNum;
    for (unsigned int index = 0; index < m_stationIdNodeNum; ++index)
    {
        StationIdNode* node = m_stationIdNodes + index;
        node->prev = 0;
        node->next = 0;
        m_companionStationIdFreeList.InsertBeforeNode(
            m_companionStationIdFreeList.Sentinel(),
            reinterpret_cast<common::ListNode*>(
                reinterpret_cast<char*>(node) + m_stationIdNodeOffset));
    }
}

void JointSessionJob::FinalizeImpl()
{
    if (m_stationIdNodeStorage == 0)
        return;

    if (m_stationIdNodes != 0)
    {
        common::ListNode* sentinel = m_companionStationIdList.Sentinel();
        common::ListNode* node = sentinel->next;
        while (node != sentinel)
        {
            common::ListNode* next = node->next;
            node->prev = 0;
            node->next = 0;
            m_companionStationIdFreeList.InsertAfterNode(
                m_companionStationIdFreeList.Sentinel(),
                reinterpret_cast<common::ListNode*>(
                    reinterpret_cast<char*>(node) + m_stationIdNodeOffset));
            node = next;
        }
        m_companionStationIdList.Init();
    }

    if (m_stationIdNodeStorage != 0)
        pead::DeleteArray(m_stationIdNodeStorage);
    m_stationIdNodeStorage = 0;
}

void JointSessionJob::NoticeHostChangeEventImpl(unsigned long long hostStationId)
{
}

void JointSessionJob::NoticeOwnerChangeEventImpl(unsigned int sessionId,
                                                 unsigned long long ownerStationId)
{
}

void JointSessionJob::NoticeLeaveEventImpl(unsigned int sessionId,
                                           unsigned long long stationId)
{
}

void JointSessionJob::NoticeUnregisterEventImpl(unsigned int sessionId)
{
}

void JointSessionJob::Cleanup()
{
    CleanupImpl();

    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();

    m_field5c = 0;
    m_fielda4 = 0;
    m_processType = 0;
    m_field9d = 0;
    m_field60 = 0;
    m_sessionDisconnectDetected = false;
    m_field9f = 0;
    m_leaveStationId = 0;
    m_fielda2 = 0;
    m_fielda3 = 0;
    m_field98 = 0;
    m_field9c = 0;
    m_fielda0 = 0;
    m_leaveStationIdAccepted = false;

    common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    common::ListNode* node = sentinel->next;
    while (node != sentinel)
    {
        common::ListNode* next = node->next;
        node->prev = 0;
        node->next = 0;
        m_companionStationIdFreeList.InsertAfterNode(
            m_companionStationIdFreeList.Sentinel(),
            reinterpret_cast<common::ListNode*>(
                reinterpret_cast<char*>(node) + m_stationIdNodeOffset));
        node = next;
    }
    m_companionStationIdList.Init();
}

bool JointSessionJob::AcceptLeaveStationId(unsigned long long stationId)
{
    if (m_leaveStationId != stationId)
        return false;
    m_leaveStationIdAccepted = true;
    return true;
}

bool JointSessionJob::IsMigrateSessionToJointSession() const
{
    return m_processType - 6 < 3;
}

bool JointSessionJob::IsMigrateJointSessionToSession() const
{
    return m_processType == 9 || m_processType == 10;
}

}}}
