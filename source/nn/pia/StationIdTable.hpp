#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_STATION_ID_TABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_STATION_ID_TABLE_HPP

#include <stdint.h>

#include "nn/pia/Common.hpp"
#include "nn/pia/ListBase.hpp"

namespace nn { namespace pia { namespace transport {


class StationIdTable
{
public:
    struct Entry
    {
        uint64_t stationId;
        uint32_t stationIndex;
        uint64_t uniqueId;
        uint8_t joinEventNotified;
    };

    struct Element
    {
        common::ListNode node;
        Entry entry;
    };

    StationIdTable();
    ~StationIdTable();
    void Clear();
    void Finalize();
    Result Initialize(unsigned int);
    Result Add(uint64_t, StationIndex, uint64_t);
    Result FindByStationId(Entry*, uint64_t) const;
    Result FindByStationIndex(Entry*, StationIndex) const;
    Result FindByUniqueId(Entry*, uint64_t) const;
    Result RemoveByStationId(uint64_t);
    Result RemoveByStationIndex(StationIndex);
    Result RemoveByUniqueId(uint64_t);
    Entry* FindUnsafe(StationIndex) const;
    StationIndex FindUnsafe(uint64_t stationId) const;
    void SetJoinEventNotifiedByStationId(uint64_t, bool);
    void SetJoinEventNotifiedByStationIndex(StationIndex, bool);
    uint8_t IsJoinEventNotifiedByStationIndex(StationIndex) const;
    uint8_t IsJoinEventNotifiedByStationId(uint64_t) const;
    unsigned int GetEntryNum() const;
    void SetAvailableEntryNum(unsigned int);
    Entry* FindByUniqueIdCore(uint64_t) const;
    Entry* FindByStationIdCore(uint64_t) const;
    void Trace(uint64_t) const;

private:
    Entry* FindByStationIndexCore(StationIndex) const;
    Result RemoveCore(Entry*);
    Result Copy(Entry*, const Entry*) const;

public:
    common::ListBase m_entries;
    common::ListBase m_availableEntries;
    int m_entryOffset;
    Element* m_elements;
    uint32_t m_elementNum;
    Element* m_storage;
    uint32_t m_availableEntryNum;
};

}}}

#endif
