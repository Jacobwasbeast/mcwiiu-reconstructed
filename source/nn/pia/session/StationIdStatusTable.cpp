#include "nn/pia/session/StationIdStatusTable.hpp"

#include <cstring>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
const unsigned short s_maxStationNum = 0x20;
const unsigned int s_maxSessionIdNum = 0x20;

enum StationIndexValue { cStationIndexReserved = 0xfe };

enum StationStateValue { cConnectedStationState = 5 };

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
        {
            common::ListNode* node = m_usedList.Sentinel()->next;
            while (node != m_usedList.Sentinel())
            {
                common::ListNode* entry = node;
                node = node->next;
                entry->prev = 0;
                entry->next = 0;
                m_freeList.InsertAfterNode(m_freeList.Sentinel(), reinterpret_cast<common::ListNode*>(reinterpret_cast<char*>(entry) + m_freeNodeOffset));
            }
            m_usedList.Init();
        }
        pead::DeleteArray(m_storage);
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
        m_storage = static_cast<unsigned long long*>(pead::allocateFromSeadHeap(sizeof(Entry) * stationNum, heap));
        if (m_storage != 0)
        {
            std::memset(m_storage, 0, sizeof(Entry) * stationNum);
            m_entries = reinterpret_cast<Entry*>(m_storage);
            m_entryNum = stationNum;
            for (unsigned short i = 0; i < stationNum; ++i)
                m_freeList.InsertBeforeNode(m_freeList.Sentinel(), &m_entries[i].node);
        }
    }
    return 0;
}

void StationIdStatusTable::ClearAllElements()
{
    common::ListNode* end = m_usedList.Sentinel();
    common::ListNode* node = end->next;
    while (node != end)
    {
        common::ListNode* entry = node;
        node = node->next;
        entry->prev = 0;
        entry->next = 0;
        m_freeList.InsertAfterNode(m_freeList.Sentinel(), reinterpret_cast<common::ListNode*>(reinterpret_cast<char*>(entry) + m_freeNodeOffset));
    }
    m_usedList.Init();
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
        Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
        {
            element->stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
            element->prepare = cPrepareStatusNone;
            element->stationId = 0;
            element->migration = false;
            element->invitation = false;
            element->host = false;
            element->connect = false;
            element->sessionId = 0;
            element->eventNotified = false;
            element->answer = cAnswerStatusNone;
            common::ListNode* entryNode = &entryOf(element)->node;
            m_usedList.EraseNode(entryNode);
            m_freeList.InsertAfterNode(m_freeList.Sentinel(), reinterpret_cast<common::ListNode*>(reinterpret_cast<char*>(entryNode) + m_freeNodeOffset));
            return 0;
        }
    }
    return static_cast<Result>(0x2c0a);
}

StationIdStatusTable::Element* StationIdStatusTable::FindElement(unsigned long long stationId)
{
    for (common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
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
    common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
        {
            element->prepare = value ? cPrepareStatusPrepared : cPrepareStatusNotPrepared;
            return;
        }
    }
}

StationIdStatusTable::PrepareCheck StationIdStatusTable::CheckAllPrepareStatus(unsigned long long* stationIds, unsigned short count) const
{
    bool filtered = false;
    if (stationIds != 0 && count != 0)
        filtered = true;
    PrepareCheck result = cPrepareCheckComplete;
    const common::ListNode* head = m_usedList.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        const Entry* entry = entryOf(node);
        if (filtered)
        {
            int i = 0;
            for (; i < count; ++i)
            {
                if (entry->element.stationId == stationIds[i])
                    break;
            }
            if (i == count)
                continue;
        }
        if (entry->element.prepare == cPrepareStatusNotPrepared)
            return cPrepareCheckNotPrepared;
        if (entry->element.prepare == cPrepareStatusNone)
            result = cPrepareCheckPending;
    }
    return result;
}

StationIdStatusTable::PrepareStatus StationIdStatusTable::CheckPrepareStatus(unsigned long long stationId) const
{
    for (const common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
    {
        const Entry* entry = entryOf(node);
        if (entry->element.stationId == stationId)
            return entry->element.prepare;
    }
    return cPrepareStatusNone;
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
        Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
            element->connect = value;
    }
}

bool StationIdStatusTable::CheckAllConnectStatus(unsigned long long* stationIds, unsigned short count) const
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        const Entry* entry = entryOf(node);
        bool found = false;
        for (unsigned short i = 0; i < count; ++i)
        {
            if (entry->element.stationId == stationIds[i])
            {
                found = true;
                break;
            }
        }
        if (found && !entry->element.connect)
            return false;
    }
    return true;
}

bool StationIdStatusTable::CheckConnectStatus(unsigned long long stationId) const
{
    for (const common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
    {
        const Entry* entry = entryOf(node);
        if (entry->element.stationId == stationId)
            return entry->element.connect;
    }
    return false;
}

bool StationIdStatusTable::SetAnswer(unsigned long long stationId, bool value)
{
    for (common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
    {
        Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
        {
            element->answer = value ? cAnswerStatusTrue : cAnswerStatusFalse;
            return true;
        }
    }
    return false;
}

StationIdStatusTable::AnswerStatus StationIdStatusTable::CheckAnswerStatus(unsigned long long stationId)
{
    for (common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
    {
        Entry* entry = entryOf(node);
        if (entry->element.stationId == stationId)
            return entry->element.answer;
    }
    return cAnswerStatusNone;
}

void StationIdStatusTable::ClearAllAnswerStatus()
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->element.answer = cAnswerStatusNone;
}

bool StationIdStatusTable::SetSendInvitationStatus(unsigned long long stationId, bool value)
{
    for (common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
    {
        Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
        {
            element->invitation = value;
            return true;
        }
    }
    return false;
}

bool StationIdStatusTable::CheckSendInvitationStatus(unsigned long long stationId) const
{
    for (const common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
    {
        const Entry* entry = entryOf(node);
        if (entry->element.stationId == stationId)
            return entry->element.invitation;
    }
    return false;
}

void StationIdStatusTable::ClearAllSendInvitationStatus()
{
    const common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->element.invitation = false;
}

bool StationIdStatusTable::SetMigrationCompleteStatus(unsigned long long stationId, bool value)
{
    for (common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
    {
        Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
        {
            element->migration = value;
            return true;
        }
    }
    return false;
}

bool StationIdStatusTable::CheckMigrationCompleteStatus(unsigned long long stationId) const
{
    for (const common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
    {
        const Entry* entry = entryOf(node);
        if (entry->element.stationId == stationId)
            return entry->element.migration;
    }
    return false;
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
        Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
            element->eventNotified = value;
    }
}

bool StationIdStatusTable::CheckEventNotifiedStatus(unsigned long long stationId) const
{
    for (const common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
    {
        const Entry* entry = entryOf(node);
        if (entry->element.stationId == stationId)
            return entry->element.eventNotified;
    }
    return false;
}

void StationIdStatusTable::CallJoinEventNotYetNotified()
{
    Session* session = g_piaSession;
    const unsigned long long localStationId = session->m_localStationId;
    if (!CheckEventNotifiedStatus(localStationId))
    {
        session->validateStationId(localStationId);
        session->JoinNotice(localStationId);
        SetEventNotifiedStatus(localStationId, true);
    }

    const unsigned long long hostStationId = session->m_hostStationId;
    if (!CheckEventNotifiedStatus(hostStationId) && CheckConnectStatus(hostStationId))
    {
        session->validateStationId(hostStationId);
        session->JoinNotice(hostStationId);
        SetEventNotifiedStatus(hostStationId, true);
    }

    common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->element;
        if (!element->eventNotified)
        {
            session->validateStationId(element->stationId);
            session->JoinNotice(element->stationId);
            element->eventNotified = true;
        }
    }
}

void StationIdStatusTable::ClearDisconnectedStationId()
{
    Session* session = g_piaSession;
    unsigned long long disconnectedStationIds[s_maxStationNum];
    unsigned short disconnectedNum = 0;
    bool localDisconnected = false;
    const unsigned long long localStationId = session->m_localStationId;

    common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->element;
        const transport::Station* station = transport::g_piaStationManager->GetStation(element->stationId);
        if (station != 0 && station->m_field20 == cConnectedStationState && element->connect)
            continue;

        if (element->stationId == localStationId)
            localDisconnected = true;
        else
            disconnectedStationIds[disconnectedNum++] = element->stationId;

        transport::StationIdTable::Entry* entry =
            transport::g_piaTransport->m_stationIdTable->FindByStationIdCore(element->stationId);
        if (entry != 0 && entry->stationIndex == cStationIndexInvalid)
            entry->stationIndex = element->stationIndex;
    }
    if (localDisconnected)
        disconnectedStationIds[disconnectedNum++] = localStationId;

    for (unsigned int i = 0; i < disconnectedNum; ++i)
        session->RemoveStationId(disconnectedStationIds[i], static_cast<StationIndex>(cStationIndexInvalid), 0, 0);
}

bool StationIdStatusTable::DisconnectionEventListenerForJointSession(void* event) const
{
    const DisconnectedStation* station = static_cast<const DisconnectedStation*>(event);
    if (station == 0 || station->stationIndex == static_cast<StationIndex>(cStationIndexReserved))
        return false;
    const Session* session = g_piaSession;
    if (session->m_fieldc0 == 0)
        return false;

    const Entry* entry = find(station->stationId);
    if (entry == 0)
        return false;

    const JointSessionJob* jointSessionJob = session->m_jointSessionJob;
    if (jointSessionJob == 0)
        return false;
    if ((jointSessionJob->m_processType == Session::cProcessTypeCreateJointSession ||
         jointSessionJob->m_processType == Session::cProcessTypeJoinJointSession ||
         jointSessionJob->m_processType == Session::cProcessTypeJoinRandomJointSession) &&
        entry->element.prepare != cPrepareStatusPrepared)
        return false;
    return true;
}

bool StationIdStatusTable::GetSessionId(unsigned long long stationId, unsigned int* sessionId) const
{
    for (const common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
    {
        const Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
        {
            *sessionId = element->sessionId;
            return true;
        }
    }
    return false;
}

void StationIdStatusTable::SetSessionHost(unsigned long long stationId, bool value)
{
    common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
        {
            element->host = value;
            return;
        }
    }
}

bool StationIdStatusTable::GetSessionHost(unsigned long long stationId, bool* value) const
{
    for (const common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
    {
        const Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
        {
            *value = element->host;
            return true;
        }
    }
    return false;
}

unsigned short StationIdStatusTable::GetStationIdNum(unsigned int sessionId) const
{
    unsigned short num = 0;
    for (const common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
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
        const Element* element = &entryOf(node)->element;
        bool found = false;
        for (int i = 0; i < num; ++i)
        {
            if (sessionIds[i] == entryOf(node)->element.sessionId)
            {
                found = true;
                break;
            }
        }
        if (!found)
            sessionIds[num++] = element->sessionId;
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
        const Element* element = &entryOf(node)->element;
        bool found = false;
        for (unsigned char i = 0; i < num; ++i)
        {
            if (sessionIds[i] == entryOf(node)->element.sessionId)
            {
                found = true;
                break;
            }
        }
        if (!found)
            sessionIds[num++] = element->sessionId;
    }
    return num;
}

void StationIdStatusTable::SetStationIndex(unsigned long long stationId, StationIndex index)
{
    common::ListNode* head = m_usedList.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
        {
            element->stationIndex = index;
            return;
        }
    }
}

bool StationIdStatusTable::GetStationIndex(unsigned long long stationId, StationIndex* index) const
{
    for (const common::ListNode* node = m_usedList.Sentinel()->next; node != m_usedList.Sentinel(); node = node->next)
    {
        const Element* element = &entryOf(node)->element;
        if (element->stationId == stationId)
        {
            *index = element->stationIndex;
            return true;
        }
    }
    return false;
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
