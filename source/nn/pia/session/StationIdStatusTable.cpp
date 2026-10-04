#include "nn/pia/session/StationIdStatusTable.hpp"

#include <cstring>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/session/Session.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
const unsigned short s_maxStationNum = 0x20;
const unsigned int s_maxSessionIdNum = 0x20;

enum StationIndexValue { cStationIndexReserved = 0xfe };

struct DisconnectedStation
{
    unsigned long long stationId;
    StationIndex stationIndex;
};
}

StationIdStatusTable::StationIdStatusTable() : m_usedList(), m_freeList(), m_freeNodeOffset(0), m_entries(0), m_entryNum(0), m_storage(0) {}

StationIdStatusTable::~StationIdStatusTable() { Finalize(); }

void StationIdStatusTable::Finalize()
{
    if (m_storage != 0)
    {
        if (m_entries != 0)
            releaseAllEntries();
        if (m_storage != 0)
        {
            pead::getArraySize(m_storage);
            pead::freeToSeadHeap(m_storage);
        }
        m_storage = 0;
    }
}

Result StationIdStatusTable::Initialize(unsigned short stationNum)
{
    if (stationNum == 0 || stationNum > s_maxStationNum)
        return static_cast<Result>(0x10c07);

    if (m_storage == 0)
    {
        pead::Heap* heap = common::HeapManager::GetHeap();
        m_storage = static_cast<Entry*>(pead::allocateFromSeadHeap(sizeof(Entry) * stationNum, heap));
        if (m_storage != 0)
        {
            std::memset(m_storage, 0, sizeof(Entry) * stationNum);
            m_entries = m_storage;
            m_entryNum = stationNum;
            for (unsigned short i = 0; i < stationNum; ++i)
                m_freeList.InsertBeforeNode(m_freeList.Sentinel(), &m_entries[i].node);
        }
    }
    return 0;
}

void StationIdStatusTable::ClearAllElements()
{
    releaseAllEntries();
}

Result StationIdStatusTable::AddElement(unsigned long long stationId, unsigned int sessionId, PrepareStatus status, bool eventNotified)
{
    common::ListNode* node = m_freeList.PopBackNode();
    if (node == 0)
        return static_cast<Result>(0xc10);

    Entry* entry = entryOf(node);
    std::memset(&entry->element, 0, sizeof(Element));
    m_usedList.InsertBeforeNode(m_usedList.Sentinel(), node);
    entry->element.stationId = stationId;
    entry->element.sessionId = sessionId;
    entry->element.prepare = status;
    entry->element.connect = true;
    entry->element.eventNotified = eventNotified;
    return 0;
}

Result StationIdStatusTable::RemoveElement(unsigned long long stationId)
{
    common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Entry* entry = entryOf(node);
        if (entry->element.stationId == stationId)
        {
            entry->element.answer = cAnswerStatusNone;
            entry->element.stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
            entry->element.sessionId = 0;
            entry->element.prepare = cPrepareStatusNone;
            entry->element.migration = false;
            entry->element.invitation = false;
            entry->element.host = false;
            entry->element.stationId = 0;
            entry->element.eventNotified = false;
            entry->element.connect = false;
            m_usedList.EraseNode(node);
            m_freeList.InsertAfterNode(m_freeList.Sentinel(), node);
            return 0;
        }
    }
    return static_cast<Result>(0x2c0a);
}

StationIdStatusTable::Element* StationIdStatusTable::FindElement(unsigned long long stationId)
{
    common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
            return element;
    }
    return 0;
}

void StationIdStatusTable::ClearAllPrepareStatus()
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->element.prepare = cPrepareStatusNone;
}

void StationIdStatusTable::SetPrepareStatus(unsigned long long stationId, bool value)
{
    Entry* entry = findMutable(stationId);
    if (entry != 0)
        entry->element.prepare = value ? cPrepareStatusPrepared : cPrepareStatusNotPrepared;
}

StationIdStatusTable::PrepareCheck StationIdStatusTable::CheckAllPrepareStatus(unsigned long long* stationIds, unsigned short count) const
{
    bool filtered = stationIds != 0 && count != 0;
    PrepareCheck result = cPrepareCheckComplete;
    const common::ListNode* head = m_usedList.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        const Element& element = entryOf(node)->element;
        if (filtered)
        {
            unsigned short i = 0;
            while (i < count && stationIds[i] != element.stationId)
                ++i;
            if (i == count)
                continue;
        }
        if (element.prepare == cPrepareStatusNotPrepared)
            return cPrepareCheckNotPrepared;
        if (element.prepare == cPrepareStatusNone)
            result = cPrepareCheckPending;
    }
    return result;
}

StationIdStatusTable::PrepareStatus StationIdStatusTable::CheckPrepareStatus(unsigned long long stationId) const
{
    const Entry* entry = find(stationId);
    if (entry == 0)
        return cPrepareStatusNone;
    return entry->element.prepare;
}

void StationIdStatusTable::ClearAllConnectStatus()
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->element.connect = false;
}

void StationIdStatusTable::SetConnect(unsigned long long stationId, bool value)
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Entry* entry = entryOf(node);
        if (entry->element.stationId == stationId)
            entry->element.connect = value;
    }
}

bool StationIdStatusTable::CheckAllConnectStatus(unsigned long long* stationIds, unsigned short count) const
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        const Element& element = entryOf(node)->element;
        for (unsigned short i = 0; i < count; ++i)
        {
            if (stationIds[i] == element.stationId)
            {
                if (!element.connect)
                    return false;
                break;
            }
        }
    }
    return true;
}

bool StationIdStatusTable::CheckConnectStatus(unsigned long long stationId) const
{
    const Entry* entry = find(stationId);
    if (entry == 0)
        return false;
    return entry->element.connect;
}

bool StationIdStatusTable::SetAnswer(unsigned long long stationId, bool value)
{
    Entry* entry = findMutable(stationId);
    if (entry == 0)
        return false;
    entry->element.answer = value ? cAnswerStatusTrue : cAnswerStatusFalse;
    return true;
}

StationIdStatusTable::AnswerStatus StationIdStatusTable::CheckAnswerStatus(unsigned long long stationId)
{
    Entry* entry = findMutable(stationId);
    if (entry == 0)
        return cAnswerStatusNone;
    return entry->element.answer;
}

void StationIdStatusTable::ClearAllAnswerStatus()
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->element.answer = cAnswerStatusNone;
}

bool StationIdStatusTable::SetSendInvitationStatus(unsigned long long stationId, bool value)
{
    Entry* entry = findMutable(stationId);
    if (entry == 0)
        return false;
    entry->element.invitation = value;
    return true;
}

bool StationIdStatusTable::CheckSendInvitationStatus(unsigned long long stationId) const
{
    const Entry* entry = find(stationId);
    if (entry == 0)
        return false;
    return entry->element.invitation;
}

void StationIdStatusTable::ClearAllSendInvitationStatus()
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->element.invitation = false;
}

bool StationIdStatusTable::SetMigrationCompleteStatus(unsigned long long stationId, bool value)
{
    Entry* entry = findMutable(stationId);
    if (entry == 0)
        return false;
    entry->element.migration = value;
    return true;
}

bool StationIdStatusTable::CheckMigrationCompleteStatus(unsigned long long stationId) const
{
    const Entry* entry = find(stationId);
    if (entry == 0)
        return false;
    return entry->element.migration;
}

void StationIdStatusTable::ClearAllMigrationCompleteStatus()
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->element.migration = false;
}

void StationIdStatusTable::SetEventNotifiedStatus(unsigned long long stationId, bool value)
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Entry* entry = entryOf(node);
        if (entry->element.stationId == stationId)
            entry->element.eventNotified = value;
    }
}

bool StationIdStatusTable::CheckEventNotifiedStatus(unsigned long long stationId) const
{
    const Entry* entry = find(stationId);
    if (entry == 0)
        return false;
    return entry->element.eventNotified;
}

void StationIdStatusTable::CallJoinEventNotYetNotified()
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->element.eventNotified = true;
}

void StationIdStatusTable::ClearDisconnectedStationId()
{
    common::ListNode* head = m_usedList.Sentinel();
    common::ListNode* node = head->next;
    while (node != head)
    {
        common::ListNode* next = node->next;
        if (entryOf(node)->element.stationIndex == static_cast<StationIndex>(cStationIndexInvalid))
            RemoveElement(entryOf(node)->element.stationId);
        node = next;
    }
}

bool StationIdStatusTable::DisconnectionEventListenerForJointSession(void* event) const
{
    const DisconnectedStation* station = static_cast<const DisconnectedStation*>(event);
    if (station == 0 || station->stationIndex == static_cast<StationIndex>(cStationIndexReserved))
        return false;
    if (g_piaSession->m_fieldc0 == 0)
        return false;

    const Entry* entry = find(station->stationId);
    if (entry == 0)
        return false;

    const JointSessionJob* jointSessionJob = g_piaSession->m_jointSessionJob;
    if (jointSessionJob == 0)
        return false;
    return !jointSessionJob->IsMigrateSessionToJointSession() || entry->element.prepare == cPrepareStatusPrepared;
}

bool StationIdStatusTable::GetSessionId(unsigned long long stationId, unsigned int* sessionId) const
{
    const Entry* entry = find(stationId);
    if (entry == 0)
        return false;
    *sessionId = entry->element.sessionId;
    return true;
}

void StationIdStatusTable::SetSessionHost(unsigned long long stationId, bool value)
{
    Entry* entry = findMutable(stationId);
    if (entry != 0)
        entry->element.host = value;
}

bool StationIdStatusTable::GetSessionHost(unsigned long long stationId, bool* value) const
{
    const Entry* entry = find(stationId);
    if (entry == 0)
        return false;
    *value = entry->element.host;
    return true;
}

unsigned short StationIdStatusTable::GetStationIdNum(unsigned int sessionId) const
{
    unsigned short num = 0;
    const common::ListNode* head = m_usedList.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        if (entryOf(node)->element.sessionId == sessionId)
            ++num;
    }
    return num;
}

unsigned char StationIdStatusTable::GetSessionIdRank(unsigned int sessionId) const
{
    unsigned int sessionIds[s_maxSessionIdNum];
    unsigned char num = 0;
    const common::ListNode* head = m_usedList.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        unsigned int id = entryOf(node)->element.sessionId;
        unsigned char i = 0;
        while (i < num && sessionIds[i] != id)
            ++i;
        if (i == num)
            sessionIds[num++] = id;
    }

    unsigned char rank = 1;
    for (unsigned char i = 0; i < num; ++i)
    {
        if (sessionIds[i] < sessionId)
            ++rank;
    }
    return rank;
}

unsigned char StationIdStatusTable::GetSessionIdArray(unsigned int* sessionIds) const
{
    unsigned char num = 0;
    const common::ListNode* head = m_usedList.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        unsigned int id = entryOf(node)->element.sessionId;
        unsigned char i = 0;
        while (i < num && sessionIds[i] != id)
            ++i;
        if (i == num)
            sessionIds[num++] = id;
    }
    return num;
}

void StationIdStatusTable::SetStationIndex(unsigned long long stationId, StationIndex index)
{
    Entry* entry = findMutable(stationId);
    if (entry != 0)
        entry->element.stationIndex = index;
}

bool StationIdStatusTable::GetStationIndex(unsigned long long stationId, StationIndex* index) const
{
    const Entry* entry = find(stationId);
    if (entry == 0)
        return false;
    *index = entry->element.stationIndex;
    return true;
}

void StationIdStatusTable::Trace(unsigned long long) const {}

unsigned int StationIdStatusTable::GetValidStationIdNum() const
{
    unsigned int num = 0;
    const common::ListNode* head = m_usedList.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
        ++num;
    return num;
}

void StationIdStatusTable::SetSessionId(unsigned long long stationId, unsigned int sessionId)
{
    Entry* entry = findMutable(stationId);
    if (entry != 0)
        entry->element.sessionId = sessionId;
}

bool StationIdStatusTable::CheckSessionHost(unsigned int sessionId) const
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        const Element& element = entryOf(node)->element;
        if (element.sessionId == sessionId)
            return element.host;
    }
    return false;
}

}}}
