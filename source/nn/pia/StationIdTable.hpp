#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_STATION_ID_TABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_STATION_ID_TABLE_HPP

#include <cstdint>

#include "nn/pia.h"
#include "nn/pia/ListBase.hpp"

namespace nn { namespace pia { namespace transport {

class StationIdTable
{
public:
    struct Entry
    {
        common::ListNode node;
        std::uint64_t stationId;
        std::uint32_t stationIndex;
        std::uint64_t uniqueId;
        bool joinEventNotified;
    };

    StationIdTable();
    ~StationIdTable();
    void Clear();
    Result Finalize();
    Result Initialize(unsigned int);
    Result Add(std::uint64_t, StationIndex, std::uint64_t);
    Result FindByStationId(Entry*, std::uint64_t) const;
    Result FindByStationIndex(Entry*, StationIndex) const;
    Result FindByUniqueId(Entry*, std::uint64_t) const;
    void RemoveByStationId(std::uint64_t);
    void SetJoinEventNotifiedByStationId(std::uint64_t, bool);
    void SetJoinEventNotifiedByStationIndex(StationIndex, bool);
    bool IsJoinEventNotifiedByStationIndex(StationIndex) const;
    bool IsJoinEventNotifiedByStationId(std::uint64_t) const;
    unsigned int GetEntryNum() const;
    void SetAvailableEntryNum(unsigned int);
    void Trace(std::uint64_t) const;

private:
    Entry* FindByStationIdCore(std::uint64_t) const;
    Entry* FindByStationIndexCore(StationIndex) const;
    Entry* FindByUniqueIdCore(std::uint64_t) const;
    void RemoveCore(Entry*);
    Result Copy(Entry*, const Entry*) const;

    common::ListBase m_entries;
    common::ListBase m_availableEntries;
    std::uint32_t m_entryOffset;
    std::uint32_t m_entryNum;
    std::uint32_t m_maxEntryNum;
    Entry* m_storage;
    std::uint32_t m_availableEntryNum;
};

}}}

#endif
