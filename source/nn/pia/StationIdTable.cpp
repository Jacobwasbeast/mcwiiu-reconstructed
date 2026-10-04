#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

#include <cstddef>
#include <cstring>

namespace nn { namespace pia { namespace transport {


StationIdTable::StationIdTable()
    : m_entryOffset(0), m_elements(0), m_elementNum(0), m_storage(0)
{
}

StationIdTable::~StationIdTable()
{
    Finalize();
}

void StationIdTable::Clear()
{
    if (m_elements != 0)
    {
        common::ListNode* sentinel = m_entries.Sentinel();
        common::ListNode* node = sentinel->next;
        while (node != sentinel)
        {
            common::ListNode* next = node->next;
            node->prev = 0;
            node->next = 0;
            m_availableEntries.InsertAfterNode(
                m_availableEntries.Sentinel(),
                reinterpret_cast<common::ListNode*>(reinterpret_cast<char*>(node) + m_entryOffset));
            node = next;
        }
        m_entries.Init();
    }
    m_availableEntryNum = m_elementNum;
}

void StationIdTable::Finalize()
{
    if (m_storage != 0)
    {
        Clear();
        pead::DeleteArray(m_storage);
        m_storage = 0;
    }
}

Result StationIdTable::Initialize(unsigned int count)
{
    if (count == 0)
        return static_cast<Result>(0x10c07);
    if (m_storage != 0)
        return static_cast<Result>(0x10c02);

    pead::Heap* heap = common::HeapManager::GetHeap();
    Element* storage = static_cast<Element*>(pead::allocateFromSeadHeap(sizeof(Element) * count, heap));
    if (storage == 0)
    {
        m_storage = 0;
        return static_cast<Result>(0x10c01);
    }
    std::memset(storage, 0, sizeof(Element) * count);
    m_storage = storage;
    m_elements = storage;
    m_elementNum = count;
    for (unsigned int index = 0; index < m_elementNum; ++index)
    {
        Element* element = m_elements + index;
        element->node.prev = 0;
        element->node.next = 0;
        m_availableEntries.InsertBeforeNode(
            m_availableEntries.Sentinel(),
            reinterpret_cast<common::ListNode*>(reinterpret_cast<char*>(element) + m_entryOffset));
    }
    m_availableEntryNum = count;
    return 0;
}

Result StationIdTable::Add(uint64_t stationId, StationIndex index,
                            uint64_t uniqueId)
{
    if (m_availableEntries.GetNodeNum() == 0)
        return static_cast<Result>(0xc10);
    common::ListNode* node = m_availableEntries.PopBackNode();
    Element* element = reinterpret_cast<Element*>(reinterpret_cast<char*>(node) - m_entryOffset);
    std::memset(&element->entry, 0, sizeof(Entry));
    m_entries.InsertBeforeNode(m_entries.Sentinel(), &element->node);
    element->entry.stationId = stationId;
    element->entry.stationIndex = static_cast<uint32_t>(index);
    element->entry.uniqueId = uniqueId;
    element->entry.joinEventNotified = false;
    return 0;
}

StationIdTable::Entry* StationIdTable::FindByStationIdCore(
    uint64_t stationId) const
{
    const common::ListNode* head = m_entries.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = reinterpret_cast<Element*>(node);
        if (element->entry.stationId == stationId)
            return &element->entry;
    }
    return 0;
}

StationIdTable::Entry* StationIdTable::FindByStationIndexCore(
    StationIndex index) const
{
    const common::ListNode* head = m_entries.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = reinterpret_cast<Element*>(node);
        if (element->entry.stationIndex == static_cast<uint32_t>(index))
            return &element->entry;
    }
    return 0;
}

StationIdTable::Entry* StationIdTable::FindByUniqueIdCore(
    uint64_t uniqueId) const
{
    const common::ListNode* head = m_entries.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = reinterpret_cast<Element*>(node);
        if (element->entry.uniqueId == uniqueId)
            return &element->entry;
    }
    return 0;
}

Result StationIdTable::Copy(Entry* destination, const Entry* source) const
{
    if (source == 0)
        return static_cast<Result>(0x2c0a);
    *destination = *source;
    return 0;
}

Result StationIdTable::FindByStationId(Entry* entry,
                                       uint64_t stationId) const
{
    return Copy(entry, FindByStationIdCore(stationId));
}

Result StationIdTable::FindByStationIndex(Entry* entry,
                                          StationIndex index) const
{
    return Copy(entry, FindByStationIndexCore(index));
}

Result StationIdTable::FindByUniqueId(Entry* entry,
                                      uint64_t uniqueId) const
{
    return Copy(entry, FindByUniqueIdCore(uniqueId));
}

Result StationIdTable::RemoveCore(Entry* entry)
{
    if (entry == 0)
        return static_cast<Result>(0x2c0a);
    Element* element = reinterpret_cast<Element*>(reinterpret_cast<char*>(entry) - offsetof(Element, entry));
    entry->stationId = 0;
    entry->stationIndex = 0xfd;
    entry->uniqueId = 0;
    entry->joinEventNotified = false;
    m_entries.EraseNode(&element->node);
    m_availableEntries.InsertAfterNode(
        m_availableEntries.Sentinel(),
        reinterpret_cast<common::ListNode*>(reinterpret_cast<char*>(element) + m_entryOffset));
    return 0;
}

Result StationIdTable::RemoveByStationId(uint64_t stationId)
{
    return RemoveCore(FindByStationIdCore(stationId));
}

void StationIdTable::SetJoinEventNotifiedByStationId(uint64_t stationId,
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

uint8_t StationIdTable::IsJoinEventNotifiedByStationIndex(
    StationIndex index) const
{
    Entry* entry = FindByStationIndexCore(index);
    return entry != 0 ? entry->joinEventNotified : 0;
}

uint8_t StationIdTable::IsJoinEventNotifiedByStationId(
    uint64_t stationId) const
{
    Entry* entry = FindByStationIdCore(stationId);
    return entry != 0 ? entry->joinEventNotified : 0;
}

unsigned int StationIdTable::GetEntryNum() const
{
    unsigned int entryNum = 0;
    const common::ListNode* sentinel = m_entries.Sentinel();
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
        ++entryNum;
    return entryNum;
}

void StationIdTable::SetAvailableEntryNum(unsigned int count)
{
    if (count > m_elementNum)
        return;
    m_availableEntryNum = count;
}


StationIndex StationIdTable::FindUnsafe(uint64_t stationId) const
{
    Entry* entry = FindByStationIdCore(stationId);
    return entry != 0 ? static_cast<StationIndex>(entry->stationIndex)
                      : static_cast<StationIndex>(0xfd);
}

}}}
