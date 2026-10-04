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
        common::ListNode node;
        uint64_t stationId;
        uint32_t stationIndex;
        uint64_t uniqueId;
        bool joinEventNotified;
    };

    StationIdTable();
    ~StationIdTable();
    void Clear();
    Result Finalize();
    Result Initialize(unsigned int);
    Result Add(uint64_t, StationIndex, uint64_t);
    Result FindByStationId(Entry*, uint64_t) const;
    Result FindByStationIndex(Entry*, StationIndex) const;
    Result FindByUniqueId(Entry*, uint64_t) const;
    void RemoveByStationId(uint64_t);
    void RemoveByStationIndex(StationIndex);
    void RemoveByUniqueId(uint64_t);
    Entry* FindUnsafe(StationIndex) const;
    StationIndex FindUnsafe(uint64_t stationId) const;
    void SetJoinEventNotifiedByStationId(uint64_t, bool);
    void SetJoinEventNotifiedByStationIndex(StationIndex, bool);
    bool IsJoinEventNotifiedByStationIndex(StationIndex) const;
    bool IsJoinEventNotifiedByStationId(uint64_t) const;
    unsigned int GetEntryNum() const;
    void SetAvailableEntryNum(unsigned int);
    Entry* FindByUniqueIdCore(uint64_t) const;
    void Trace(uint64_t) const;

private:
    Entry* FindByStationIdCore(uint64_t) const;
    Entry* FindByStationIndexCore(StationIndex) const;
    void RemoveCore(Entry*);
    Result Copy(Entry*, const Entry*) const;

public:
    common::ListBase m_entries;
    common::ListBase m_availableEntries;
    uint32_t m_entryOffset;
    uint32_t m_entryNum;
    uint32_t m_maxEntryNum;
    Entry* m_storage;
    uint32_t m_availableEntryNum;
};

}}}

#endif
