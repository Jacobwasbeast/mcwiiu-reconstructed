#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

#include <cstring>

namespace nn { namespace pia { namespace transport {

StationIdTable::StationIdTable()
    : m_entryOffset(0), m_entryNum(0), m_maxEntryNum(0), m_storage(0),
      m_availableEntryNum(0)
{
}

StationIdTable::~StationIdTable()
{
    Finalize();
}

void StationIdTable::Clear()
{
    while (m_entries.Sentinel()->next != m_entries.Sentinel())
    {
        Entry* entry = reinterpret_cast<Entry*>(m_entries.PopFrontNode());
        m_availableEntries.InsertAfterNode(m_availableEntries.Sentinel(),
                                            &entry->node);
    }
    m_entryNum = 0;
    m_availableEntryNum = m_maxEntryNum;
}

Result StationIdTable::Finalize()
{
    if (m_storage != 0)
    {
        Clear();
        pead::freeToSeadHeap(m_storage);
        m_storage = 0;
    }
    return 0;
}

Result StationIdTable::Initialize(unsigned int count)
{
    if (count == 0)
        return static_cast<Result>(0x10c07);
    if (m_storage != 0)
        return static_cast<Result>(0x10c02);

    m_maxEntryNum = count;
    m_availableEntryNum = count;
    pead::Heap* heap = common::HeapManager::GetHeap();
    m_storage = static_cast<Entry*>(pead::allocateFromSeadHeap(
        sizeof(Entry) * count, heap));
    if (m_storage == 0)
    {
        m_maxEntryNum = 0;
        m_availableEntryNum = 0;
        return static_cast<Result>(0x10c01);
    }
    for (unsigned int i = 0; i < count; ++i)
    {
        std::memset(&m_storage[i], 0, sizeof(Entry));
        m_availableEntries.InsertAfterNode(m_availableEntries.Sentinel(),
                                            &m_storage[i].node);
    }
    return 0;
}

Result StationIdTable::Add(std::uint64_t stationId, StationIndex index,
                            std::uint64_t uniqueId)
{
    common::ListNode* node = m_availableEntries.PopBackNode();
    if (node == 0)
        return static_cast<Result>(0xc10);
    Entry* entry = reinterpret_cast<Entry*>(node);
    std::memset(entry, 0, sizeof(Entry));
    entry->stationId = stationId;
    entry->stationIndex = static_cast<std::uint32_t>(index);
    entry->uniqueId = uniqueId;
    m_entries.InsertBeforeNode(m_entries.Sentinel(), node);
    ++m_entryNum;
    --m_availableEntryNum;
    return 0;
}

StationIdTable::Entry* StationIdTable::FindByStationIdCore(
    std::uint64_t stationId) const
{
    const common::ListNode* head = m_entries.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Entry* entry = reinterpret_cast<Entry*>(node);
        if (entry->stationId == stationId)
            return entry;
    }
    return 0;
}

StationIdTable::Entry* StationIdTable::FindByStationIndexCore(
    StationIndex index) const
{
    const common::ListNode* head = m_entries.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Entry* entry = reinterpret_cast<Entry*>(node);
        if (entry->stationIndex == static_cast<std::uint32_t>(index))
            return entry;
    }
    return 0;
}

StationIdTable::Entry* StationIdTable::FindByUniqueIdCore(
    std::uint64_t uniqueId) const
{
    const common::ListNode* head = m_entries.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Entry* entry = reinterpret_cast<Entry*>(node);
        if (entry->uniqueId == uniqueId)
            return entry;
    }
    return 0;
}

Result StationIdTable::Copy(Entry* destination, const Entry* source) const
{
    if (destination == 0)
        return static_cast<Result>(0x10c07);
    if (source == 0)
        return static_cast<Result>(0x2c0a);
    std::memcpy(destination, source, sizeof(Entry));
    return 0;
}

Result StationIdTable::FindByStationId(Entry* entry,
                                       std::uint64_t stationId) const
{
    return Copy(entry, FindByStationIdCore(stationId));
}

Result StationIdTable::FindByStationIndex(Entry* entry,
                                          StationIndex index) const
{
    return Copy(entry, FindByStationIndexCore(index));
}

Result StationIdTable::FindByUniqueId(Entry* entry,
                                      std::uint64_t uniqueId) const
{
    return Copy(entry, FindByUniqueIdCore(uniqueId));
}

void StationIdTable::RemoveCore(Entry* entry)
{
    if (entry == 0)
        return;
    m_entries.EraseNode(&entry->node);
    m_availableEntries.InsertAfterNode(m_availableEntries.Sentinel(),
                                        &entry->node);
    --m_entryNum;
    ++m_availableEntryNum;
}

void StationIdTable::RemoveByStationId(std::uint64_t stationId)
{
    RemoveCore(FindByStationIdCore(stationId));
}

void StationIdTable::SetJoinEventNotifiedByStationId(std::uint64_t stationId,
                                                     bool notified)
{
    Entry* entry = FindByStationIdCore(stationId);
    if (entry != 0)
        entry->joinEventNotified = notified;
}

void StationIdTable::SetJoinEventNotifiedByStationIndex(StationIndex index,
                                                        bool notified)
{
    Entry* entry = FindByStationIndexCore(index);
    if (entry != 0)
        entry->joinEventNotified = notified;
}

bool StationIdTable::IsJoinEventNotifiedByStationIndex(
    StationIndex index) const
{
    Entry* entry = FindByStationIndexCore(index);
    return entry != 0 && entry->joinEventNotified;
}

bool StationIdTable::IsJoinEventNotifiedByStationId(
    std::uint64_t stationId) const
{
    Entry* entry = FindByStationIdCore(stationId);
    return entry != 0 && entry->joinEventNotified;
}

unsigned int StationIdTable::GetEntryNum() const
{
    return m_entryNum;
}

void StationIdTable::SetAvailableEntryNum(unsigned int count)
{
    if (count <= m_maxEntryNum)
        m_availableEntryNum = count;
}

void StationIdTable::Trace(std::uint64_t) const
{
}

}}}
