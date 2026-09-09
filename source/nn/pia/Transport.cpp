#include "nn/pia/Transport.hpp"
namespace nn { namespace pia { namespace transport {
Transport* g_piaTransport = 0;
Transport::Transport()
    : m_transportStatus(static_cast<Result>(0x10c08)), m_stationNum(0),
      m_connectionError(0), m_stationIdTable(0)
{
}

Result Transport::ConvertToStationIndex(StationIndex* index,
                                         std::uint64_t stationId) const
{
    if (index == 0)
        return static_cast<Result>(0x10c07);
    if (m_transportStatus == static_cast<Result>(0x10c08))
        return static_cast<Result>(0x10c08);
    if (m_transportStatus == static_cast<Result>(0x4c11))
        return static_cast<Result>(0x4c11);
    if (stationId == 0)
    {
        *index = static_cast<StationIndex>(0xfd);
        return 0;
    }
    StationIdTable::Entry entry;
    const Result result = m_stationIdTable->FindByStationId(&entry, stationId);
    if (result != 0)
        return result;
    *index = entry.stationIndex;
    return 0;
}
StationIndex Conv2StationIndex(std::uint64_t stationId)
{
    StationIndex index;
    if (g_piaTransport != 0 &&
        g_piaTransport->ConvertToStationIndex(&index, stationId) == 0)
        return index;
    return static_cast<StationIndex>(0xfd);
}
}}}
