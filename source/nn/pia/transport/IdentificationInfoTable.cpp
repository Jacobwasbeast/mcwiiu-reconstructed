#include "nn/pia/transport/IdentificationInfoTable.hpp"

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>
#include <string.h>

namespace nn { namespace pia { namespace transport {

namespace
{
IdentificationInfoTable* g_identificationInfoTable = 0;
void* g_identificationInfoTableStorage = 0;

inline void CopyToken(IdentificationInfo* destination, const IdentificationInfo* source)
{
    memcpy(destination->m_field00.m_token, source->m_field00.m_token, sizeof(destination->m_field00.m_token));
}

inline void CopyCounts(IdentificationInfo* destination, const IdentificationInfo* source)
{
    destination->m_field109 = source->m_field109;
    destination->m_playerCount = source->m_playerCount;
    destination->m_participantCount = source->m_participantCount;
    destination->m_field10c = source->m_field10c;
}

inline bool SameCountsAndToken(const IdentificationInfo* stored, const IdentificationInfo* info)
{
    if (stored->m_field109 != info->m_field109 || stored->m_playerCount != info->m_playerCount ||
        stored->m_participantCount != info->m_participantCount ||
        stored->m_field10c != info->m_field10c)
        return false;
    for (unsigned int i = 0; i < sizeof(stored->m_field00.m_token); ++i)
        if (stored->m_field00.m_token[i] != info->m_field00.m_token[i]) return false;
    return true;
}
}

IdentificationInfoTable::IdentificationInfoTable()
    : m_count(0), m_criticalSection(-1)
{
    ClearIdentificationInfo(this);
}

IdentificationInfoTable::~IdentificationInfoTable()
{
    Finalize();
}

Result IdentificationInfoTable::CreateInstance()
{
    if (IsInitialized() == 0) return 0x10c0c;
    if (IsInSetupMode() == 0) return 0x10c08;
    if (g_identificationInfoTable != 0) return 0x10c0f;

    pead::Heap* heap = common::HeapManager::GetHeap();
    void* storage = pead::allocateFromSeadHeap(sizeof(IdentificationInfoTable), heap, 8);
    if (storage != 0)
    {
        unsigned char* bytes = static_cast<unsigned char*>(storage);
        for (unsigned int remaining = sizeof(IdentificationInfoTable); remaining != 0; --remaining)
        {
            new (bytes) unsigned char();
            ++bytes;
        }
        g_identificationInfoTableStorage = storage;
        g_identificationInfoTable = new (storage) IdentificationInfoTable();
        return 0;
    }
    g_identificationInfoTableStorage = 0;
    return 0x10c01;
}

void IdentificationInfoTable::DestroyInstance()
{
    if (g_identificationInfoTable != 0)
    {
        g_identificationInfoTable->~IdentificationInfoTable();
        g_identificationInfoTable = 0;
    }
    void* storage = g_identificationInfoTableStorage;
    if (storage != 0)
    {
        pead::getArraySize(g_identificationInfoTableStorage);
        pead::freeToSeadHeap(storage);
        g_identificationInfoTableStorage = 0;
    }
}

IdentificationInfoTable* IdentificationInfoTable::GetInstance()
{
    return g_identificationInfoTable;
}

void IdentificationInfoTable::Finalize()
{
    while (m_pool.m_usedSlots.Sentinel()->next != m_pool.m_usedSlots.Sentinel())
    {
        Slot* slot = reinterpret_cast<Slot*>(m_pool.m_usedSlots.Sentinel()->prev);
        slot->GetRecord()->~Record();
        m_pool.m_usedSlots.EraseNode(&slot->m_node);
        m_pool.m_freeSlots.InsertAfterNode(
            m_pool.m_freeSlots.Sentinel(),
            reinterpret_cast<common::ListNode*>(
                reinterpret_cast<char*>(slot) + m_pool.m_freeSlots.m_offset));
    }
}

Result IdentificationInfoTable::Initialize(unsigned short count)
{
    if (count > 32) return 0x10c07;
    m_count = count;
    for (unsigned int i = 0; i < m_count; ++i)
    {
        if (m_pool.m_freeSlots.Sentinel()->next == m_pool.m_freeSlots.Sentinel()) return 0x10c08;
        common::ListNode* node = m_pool.m_freeSlots.PopBackNode();
        Slot* slot = reinterpret_cast<Slot*>(reinterpret_cast<char*>(node) - m_pool.m_freeSlots.m_offset);
        new (slot->GetRecord()) Record();
        m_pool.m_usedSlots.InsertBeforeNode(m_pool.m_usedSlots.Sentinel(), &slot->m_node);
        slot->GetRecord()->m_station = 0;
        slot->GetRecord()->m_state = 0;
        ClearIdentificationInfo(&slot->GetRecord()->m_info);
    }
    return 0;
}

void IdentificationInfoTable::ClearIdentificationInfo(IdentificationInfo* info)
{
    memset(info->m_field00.m_token, 0, sizeof(info->m_field00.m_token));
    info->m_fieldC8[0].Clear();
    info->m_field60[0].Clear();
    info->m_field108 = 0xff;
    memset(info->m_field20.m_bytes, 0, sizeof(info->m_field20.m_bytes));
    info->m_playerCount = 0;
    info->m_field109 = 0;
    info->m_principalId = 0;
    info->m_field10c = 0;
    info->m_participantCount = 0;
}

void IdentificationInfoTable::SetLocalIdentificationInfo(const IdentificationInfo* info)
{
    if (info != 0)
    {
        m_field00 = info->m_field00;
        m_fieldC8[0] = info->m_fieldC8[0];
        m_field60[0] = info->m_field60[0];
        m_field108 = info->m_field108;
        m_field20 = info->m_field20;
        m_principalId = info->m_principalId;
        m_fieldC8[0].Trace(0x4000000);
        m_field60[0].Trace(0x4000000);
        CopyCounts(this, info);
    }
}

Result IdentificationInfoTable::GetLocalIdentificationInfo(IdentificationInfo* info)
{
    if (info == 0) return 0x10c07;
    info->m_field00 = m_field00;
    info->m_fieldC8[0] = m_fieldC8[0];
    info->m_field60[0] = m_field60[0];
    info->m_field108 = m_field108;
    info->m_field20 = m_field20;
    info->m_principalId = m_principalId;
    CopyCounts(info, this);
    return 0;
}

void IdentificationInfoTable::ClearLocalIdentificationInfo()
{
    ClearIdentificationInfo(this);
}

Result IdentificationInfoTable::AddToTable(const Station* station, const IdentificationInfo* info,
                                           unsigned char kind)
{
    if (station == 0 || info == 0 || kind > 2) return 0x10c07;
    station->Trace(0x8000);
    m_criticalSection.Lock();
    common::ListNode* first = m_pool.m_usedSlots.Sentinel()->next;
    for (common::ListNode* node = first; node != m_pool.m_usedSlots.Sentinel(); node = node->next)
    {
        Record& record = *reinterpret_cast<Slot*>(node)->GetRecord();
        if (record.m_station != station) continue;
        if (kind == 0)
        {
            record.m_state = 3;
            memcpy(record.m_info.m_field00.m_token, info->m_field00.m_token, sizeof(record.m_info.m_field00.m_token));
            record.m_info.m_fieldC8[0].Copy(info->m_fieldC8[0]);
            record.m_info.m_field60[0].Copy(info->m_field60[0]);
            record.m_info.m_field108 = info->m_field108;
            memcpy(record.m_info.m_field20.m_bytes, info->m_field20.m_bytes, sizeof(record.m_info.m_field20.m_bytes));
            record.m_info.m_principalId = info->m_principalId;
            CopyCounts(&record.m_info, info);
        }
        else if (kind == 1)
        {
            if ((record.m_state & 2) != 0 && !SameCountsAndToken(&record.m_info, info))
            {
                m_criticalSection.Unlock();
                return 0x10c08;
            }
            record.m_state |= 1;
            memcpy(record.m_info.m_field00.m_token, info->m_field00.m_token, sizeof(record.m_info.m_field00.m_token));
            record.m_info.m_fieldC8[0].Copy(info->m_fieldC8[0]);
            record.m_info.m_field60[0].Copy(info->m_field60[0]);
            record.m_info.m_field108 = info->m_field108;
            memcpy(record.m_info.m_field20.m_bytes, info->m_field20.m_bytes, sizeof(record.m_info.m_field20.m_bytes));
            record.m_info.m_principalId = info->m_principalId;
            CopyCounts(&record.m_info, info);
        }
        else
        {
            if ((record.m_state & 1) != 0 && !SameCountsAndToken(&record.m_info, info))
            {
                m_criticalSection.Unlock();
                return 0x10c08;
            }
            record.m_state |= 2;
            CopyToken(&record.m_info, info);
            CopyCounts(&record.m_info, info);
        }
        m_criticalSection.Unlock();
        return 0;
    }
    for (common::ListNode* node = first; node != m_pool.m_usedSlots.Sentinel(); node = node->next)
    {
        Record& record = *reinterpret_cast<Slot*>(node)->GetRecord();
        if (record.m_station != 0) continue;
        record.m_station = station;
        CopyToken(&record.m_info, info);
        if (kind == 0)
        {
            record.m_state = 3;
            record.m_info.m_fieldC8[0].Copy(info->m_fieldC8[0]);
            record.m_info.m_field60[0].Copy(info->m_field60[0]);
            record.m_info.m_field108 = info->m_field108;
            memcpy(record.m_info.m_field20.m_bytes, info->m_field20.m_bytes, sizeof(record.m_info.m_field20.m_bytes));
            record.m_info.m_principalId = info->m_principalId;
        }
        else if (kind == 1)
        {
            record.m_state |= 1;
            record.m_info.m_fieldC8[0].Copy(info->m_fieldC8[0]);
            record.m_info.m_field60[0].Copy(info->m_field60[0]);
            record.m_info.m_field108 = info->m_field108;
            memcpy(record.m_info.m_field20.m_bytes, info->m_field20.m_bytes, sizeof(record.m_info.m_field20.m_bytes));
            record.m_info.m_principalId = info->m_principalId;
        }
        else
        {
            record.m_state |= 2;
        }
        CopyCounts(&record.m_info, info);
        m_criticalSection.Unlock();
        return 0;
    }
    m_criticalSection.Unlock();
    return 0x10c08;
}

IdentificationInfo* IdentificationInfoTable::GetIdentificationInfoPtr(const Station* station)
{
    m_criticalSection.Lock();
    for (common::ListNode* node = m_pool.m_usedSlots.Sentinel()->next;
         node != m_pool.m_usedSlots.Sentinel(); node = node->next)
    {
        Slot* slot = reinterpret_cast<Slot*>(node);
        if (slot->GetRecord()->m_station == station && slot->GetRecord()->m_state == 3)
        {
            m_criticalSection.Unlock();
            return &slot->GetRecord()->m_info;
        }
    }
    m_criticalSection.Unlock();
    return 0;
}

Result IdentificationInfoTable::GetIdentificationInfo(const Station* station,
                                                      IdentificationInfo* info)
{
    if (station == 0 || info == 0) return 0x10c07;
    IdentificationInfo* found = GetIdentificationInfoPtr(station);
    if (found == 0) return 0x2c09;
    memcpy(info->m_field00.m_token, found->m_field00.m_token, sizeof(info->m_field00.m_token));
    info->m_fieldC8[0].Copy(found->m_fieldC8[0]);
    info->m_field60[0].Copy(found->m_field60[0]);
    info->m_field108 = found->m_field108;
    memcpy(info->m_field20.m_bytes, found->m_field20.m_bytes, sizeof(info->m_field20.m_bytes));
    info->m_principalId = found->m_principalId;
    CopyCounts(info, found);
    return 0;
}

IdentificationInfo* IdentificationInfoTable::GetIdentificationInfoPtr(unsigned long long principalId)
{
    m_criticalSection.Lock();
    for (common::ListNode* node = m_pool.m_usedSlots.Sentinel()->next;
         node != m_pool.m_usedSlots.Sentinel(); node = node->next)
    {
        Slot* slot = reinterpret_cast<Slot*>(node);
        if (slot->GetRecord()->m_state == 3 &&
            slot->GetRecord()->m_info.m_principalId == principalId)
        {
            m_criticalSection.Unlock();
            return &slot->GetRecord()->m_info;
        }
    }
    m_criticalSection.Unlock();
    return 0;
}

Result IdentificationInfoTable::GetIdentificationInfo(unsigned long long principalId,
                                                      IdentificationInfo* info)
{
    if (info == 0) return 0x10c07;
    IdentificationInfo* found = GetIdentificationInfoPtr(principalId);
    if (found == 0) return 0x2c09;
    memcpy(info->m_field00.m_token, found->m_field00.m_token, sizeof(info->m_field00.m_token));
    info->m_fieldC8[0].Copy(found->m_fieldC8[0]);
    info->m_field60[0].Copy(found->m_field60[0]);
    info->m_field108 = found->m_field108;
    memcpy(info->m_field20.m_bytes, found->m_field20.m_bytes, sizeof(info->m_field20.m_bytes));
    info->m_principalId = found->m_principalId;
    CopyCounts(info, found);
    return 0;
}

Result IdentificationInfoTable::GetPlayerCount(const Station* station, unsigned char* count)
{
    if (station == 0) return 0x10c07;
    if (count == 0) return 0x10c07;
    IdentificationInfo* found = GetIdentificationInfoPtr(station);
    if (found == 0) return 0x2c09;
    *count = found->m_playerCount;
    return 0;
}

Result IdentificationInfoTable::GetParticipantCount(const Station* station, unsigned char* count)
{
    if (station == 0) return 0x10c07;
    if (count == 0) return 0x10c07;
    IdentificationInfo* found = GetIdentificationInfoPtr(station);
    if (found == 0) return 0x2c09;
    *count = found->m_participantCount;
    return 0;
}

void IdentificationInfoTable::EraseFromTable(const Station* station)
{
    if (station == 0) return;
    m_criticalSection.Lock();
    for (common::ListNode* node = m_pool.m_usedSlots.Sentinel()->next;
         node != m_pool.m_usedSlots.Sentinel(); node = node->next)
    {
        Slot* slot = reinterpret_cast<Slot*>(node);
        if (slot->GetRecord()->m_station == station)
        {
            slot->GetRecord()->m_station = 0;
            slot->GetRecord()->m_state = 0;
            ClearIdentificationInfo(&slot->GetRecord()->m_info);
            m_criticalSection.Unlock();
            return;
        }
    }
    m_criticalSection.Unlock();
}

void IdentificationInfoTable::ClearTable()
{
    m_criticalSection.Lock();
    for (common::ListNode* node = m_pool.m_usedSlots.Sentinel()->next;
         node != m_pool.m_usedSlots.Sentinel(); node = node->next)
    {
        Slot* slot = reinterpret_cast<Slot*>(node);
        slot->GetRecord()->m_station = 0;
        slot->GetRecord()->m_state = 0;
        ClearIdentificationInfo(&slot->GetRecord()->m_info);
    }
    m_criticalSection.Unlock();
}

void IdentificationInfoTable::Trace(unsigned long long) const
{
}

}}}
