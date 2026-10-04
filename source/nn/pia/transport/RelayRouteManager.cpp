#include "nn/pia/transport/RelayRouteManager.hpp"

#include <cstring>
#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace transport {

namespace {

struct StationRank
{
    unsigned char station;
    unsigned char brokenRouteCount;
};

struct RttCandidate
{
    unsigned int station;
    unsigned int rtt;
};

unsigned short g_rttMax;
unsigned short g_relayNumMax;

void sortStationRanks(StationRank* ranks, unsigned int count)
{
    for (unsigned int pass = count - 1; pass != 0; --pass)
    {
        for (unsigned int index = 0; index < pass; ++index)
        {
            if (ranks[index + 1].brokenRouteCount < ranks[index].brokenRouteCount)
            {
                unsigned char* current = &ranks[index].station;
                unsigned char* next = &ranks[index + 1].station;
                for (unsigned int byteIndex = 0; byteIndex < sizeof(StationRank); ++byteIndex)
                {
                    const unsigned char swapped = current[byteIndex];
                    current[byteIndex] = next[byteIndex];
                    next[byteIndex] = swapped;
                }
            }
        }
    }
}

template <typename T>
inline T* allocateHeapArray(unsigned int count)
{
    T* array = static_cast<T*>(
        pead::allocateFromSeadHeap(sizeof(T) * count, common::HeapManager::GetHeap()));
    if (array != 0)
    {
        for (unsigned int index = 0; index < count; ++index)
            new (array + index) T();
    }
    return array;
}

}

RelayRouteManager::RelayRouteManager()
    : m_rttTable(0), m_routeTable(0), m_relayNumTable(0), m_directStationBitmaps(0),
      m_relayEmulationTable(0), m_failureReasonTable(0), m_rttMax(500), m_relayNumMax(0x400),
      m_unknown1c(0), m_unknown20(0), m_unknown24(0xfd), m_routeBitCount(5),
      m_invalidRoute(0xff), m_joiningStationBitmap(0), m_unknown30(0),
      m_originalRouteTable(0)
{
}

RelayRouteManager::~RelayRouteManager()
{
}

Result RelayRouteManager::Initialize(unsigned int stationNum, bool relayEmulationEnabled)
{
    if (stationNum == 0 || stationNum > 0x20)
        return 0x10c07;

    const unsigned int tableSize = stationNum * stationNum;
    m_rttTable = allocateHeapArray<unsigned short>(tableSize);
    if (m_rttTable == 0)
        return 0x10c01;

    std::memset(m_rttTable, 0, tableSize * 2);
    m_routeTable = common::NewArray<unsigned char>(tableSize);
    if (m_routeTable == 0)
        return 0x10c01;

    unsigned int routeIndex = 0;
    for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
         --sourceCount, ++source)
    {
        for (unsigned int destination = 0, destinationCount = stationNum; destinationCount != 0;
             --destinationCount, ++destination)
            m_routeTable[routeIndex++] = static_cast<unsigned char>(source);
    }

    m_relayNumTable = common::NewArray<unsigned char>(stationNum);
    if (m_relayNumTable == 0)
        return 0x10c01;

    std::memset(m_relayNumTable, 0, stationNum);
    m_directStationBitmaps = allocateHeapArray<unsigned int>(stationNum);
    if (m_directStationBitmaps == 0)
        return 0x10c01;

    std::memset(m_directStationBitmaps, 0, stationNum << 2);
    m_relayEmulationEnabled = relayEmulationEnabled;
    if (relayEmulationEnabled)
    {
        m_relayEmulationTable = common::NewArray<unsigned char>(tableSize);
        if (m_relayEmulationTable == 0)
            return 0x10c01;

        std::memset(m_relayEmulationTable, 0, tableSize);
    }

    m_failureReasonTable = common::NewArray<unsigned char>(stationNum);
    if (m_failureReasonTable == 0)
        return 0x10c01;

    std::memset(m_failureReasonTable, 0, stationNum);
    m_originalRouteTable = common::NewArray<unsigned char>(tableSize);
    if (m_originalRouteTable == 0)
        return 0x10c01;

    std::memcpy(m_originalRouteTable, m_routeTable, tableSize);
    return 0;
}

void RelayRouteManager::Finalize()
{
    if (m_rttTable != 0)
    {
        pead::getArraySize(m_rttTable);
        pead::freeToSeadHeap(m_rttTable);
        m_rttTable = 0;
    }
    if (m_routeTable != 0)
    {
        common::DeleteArray<unsigned char>(m_routeTable);
        m_routeTable = 0;
    }
    if (m_relayNumTable != 0)
    {
        common::DeleteArray<unsigned char>(m_relayNumTable);
        m_relayNumTable = 0;
    }
    if (m_directStationBitmaps != 0)
    {
        pead::getArraySize(m_directStationBitmaps);
        pead::freeToSeadHeap(m_directStationBitmaps);
        m_directStationBitmaps = 0;
    }
    if (m_relayEmulationEnabled && m_relayEmulationTable != 0)
    {
        common::DeleteArray<unsigned char>(m_relayEmulationTable);
        m_relayEmulationTable = 0;
    }
    if (m_failureReasonTable != 0)
    {
        common::DeleteArray<unsigned char>(m_failureReasonTable);
        m_failureReasonTable = 0;
    }
    if (m_originalRouteTable != 0)
    {
        common::DeleteArray<unsigned char>(m_originalRouteTable);
        m_originalRouteTable = 0;
    }
}

void RelayRouteManager::CalcStationType(unsigned int stationBitmap,
                                        unsigned int* stayStationBitmap,
                                        unsigned int* leaveStationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    *stayStationBitmap = 0;
    *leaveStationBitmap = 0;
    for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
         --stationCount, ++station)
    {
        const unsigned int stationFlag = 1 << station;
        const bool isJoining = (m_joiningStationBitmap & stationFlag) != 0;
        const bool isAbsent = (stationBitmap & stationFlag) == 0;
        bool hasRtt = false;
        for (unsigned int other = 0, otherCount = stationNum; otherCount != 0;
             --otherCount, ++other)
        {
            if (m_rttTable[station * stationNum + other] != 0)
            {
                hasRtt = true;
                break;
            }
        }

        if (hasRtt)
        {
            if (isJoining)
            {
                if (isAbsent)
                    *stayStationBitmap |= stationFlag;
                else
                    *leaveStationBitmap |= stationFlag;
            }
        }
        else if (isAbsent)
        {
            if (isJoining)
            {
                if (!m_relayEmulationEnabled || station != m_unknown24)
                    *leaveStationBitmap |= stationFlag;
                else
                    *stayStationBitmap |= stationFlag;
            }
        }
        else if (m_relayEmulationEnabled)
        {
            for (unsigned int other = 0, otherCount = stationNum; otherCount != 0;
                 --otherCount, ++other)
            {
                if (m_relayEmulationTable[station * stationNum + other] == 1)
                {
                    *leaveStationBitmap |= stationFlag;
                    break;
                }
            }
        }
    }
}

void RelayRouteManager::ProcLeaveStation(unsigned int leaveStationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
         --stationCount, ++station)
    {
        if ((leaveStationBitmap & (1 << station)) != 0)
        {
            for (unsigned int other = 0, otherCount = stationNum; otherCount != 0;
                 --otherCount, ++other)
            {
                m_routeTable[station * stationNum + other] = static_cast<unsigned char>(station);
                m_routeTable[other * stationNum + station] = static_cast<unsigned char>(other);
            }
        }
    }

    for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
         --sourceCount, ++source)
    {
        for (unsigned int destination = 0, destinationCount = stationNum; destinationCount != 0;
             --destinationCount, ++destination)
        {
            const unsigned int relay = m_routeTable[source * stationNum + destination];
            if (source != relay && destination != relay &&
                (leaveStationBitmap & (1 << relay)) != 0)
                m_routeTable[source * stationNum + destination] = m_invalidRoute;
        }
    }
}

void RelayRouteManager::CalcRelayNumTable()
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    std::memset(m_relayNumTable, 0, stationNum);
    for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
         --sourceCount, ++source)
    {
        for (unsigned int destination = 0, destinationCount = stationNum; destinationCount != 0;
             --destinationCount, ++destination)
        {
            const unsigned int relay = m_routeTable[source * stationNum + destination];
            if (relay != source && relay != destination && relay < stationNum)
                ++m_relayNumTable[relay];
        }
    }
}

bool RelayRouteManager::SwitchRelay(unsigned int relayStation, unsigned int stationBitmap,
                                    unsigned short rttLimit, unsigned short relayNumLimit)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    if (stationNum > 1)
    {
        for (unsigned int source = 1, sourceCount = stationNum - 1; sourceCount != 0;
             --sourceCount, ++source)
        {
            if (source == relayStation)
                continue;

            for (unsigned int destination = 0, destinationCount = source; destinationCount != 0;
                 --destinationCount, ++destination)
            {
                if (destination == relayStation ||
                    m_routeTable[source * stationNum + destination] != relayStation)
                    continue;

                for (unsigned int relay = 0, relayCount = stationNum; relayCount != 0;
                     --relayCount, ++relay)
                {
                    if ((stationBitmap & (1 << relay)) != 0 && relay != source &&
                        relay != destination && relay != relayStation &&
                        IsRelayEnabled(source, relay, destination, 0) &&
                        static_cast<int>(m_rttTable[source * stationNum + relay] +
                                         m_rttTable[relay * stationNum + destination]) <=
                            rttLimit &&
                        static_cast<int>(m_relayNumTable[relay] + 2) <= relayNumLimit)
                    {
                        m_routeTable[source * stationNum + destination] =
                            static_cast<unsigned char>(relay);
                        m_routeTable[destination * stationNum + source] =
                            static_cast<unsigned char>(relay);
                        m_relayNumTable[relay] += 2;
                        m_relayNumTable[relayStation] -= 2;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool RelayRouteManager::CalcRelayRouteWithSwitchingRelay(unsigned int source,
                                                         unsigned int destination,
                                                         unsigned int stationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int candidateBitmap = 0;
    for (unsigned int relay = 0, relayCount = stationNum; relayCount != 0; --relayCount, ++relay)
    {
        const unsigned int relayFlag = 1 << relay;
        if ((stationBitmap & relayFlag) != 0)
        {
            const unsigned short sourceRtt = m_rttTable[source * stationNum + relay];
            if (sourceRtt != 0)
            {
                const unsigned short destinationRtt = m_rttTable[relay * stationNum + destination];
                if (destinationRtt != 0)
                {
                    if (sourceRtt + destinationRtt <= m_rttMax)
                    {
                        if (static_cast<unsigned short>(m_relayNumTable[relay]) <= m_relayNumMax)
                            candidateBitmap |= relayFlag;
                    }
                }
            }
        }
    }

    for (unsigned int relay = 0, relayCount = stationNum; relayCount != 0; --relayCount, ++relay)
    {
        if ((candidateBitmap & (1 << relay)) != 0 &&
            SwitchRelay(relay, stationBitmap, m_rttMax, m_relayNumMax))
        {
            m_routeTable[source * stationNum + destination] = static_cast<unsigned char>(relay);
            m_routeTable[destination * stationNum + source] = static_cast<unsigned char>(relay);
            m_relayNumTable[relay] += 2;
            return true;
        }
    }
    return false;
}

bool RelayRouteManager::DecideBrokenRouteRelayStation(unsigned int stationBitmap,
                                                      unsigned int* refusedStationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    StationRank ranks[32];
    StationRank relayCandidates[32];
    for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
         --stationCount, ++station)
    {
        unsigned char brokenRouteCount = 0;
        for (unsigned int other = 0, otherCount = stationNum; otherCount != 0;
             --otherCount, ++other)
        {
            if (m_routeTable[station * stationNum + other] == m_invalidRoute)
                ++brokenRouteCount;
            if (m_routeTable[other * stationNum + station] == m_invalidRoute)
                ++brokenRouteCount;
        }
        ranks[station].station = static_cast<unsigned char>(station);
        ranks[station].brokenRouteCount = brokenRouteCount;
    }

    sortStationRanks(ranks, stationNum);
    for (unsigned int index = 0, indexCount = stationNum; indexCount != 0; --indexCount, ++index)
    {
        relayCandidates[index].station = ranks[index].station;
        relayCandidates[index].brokenRouteCount = ranks[index].brokenRouteCount;
    }

    for (unsigned int rankIndex = 1; rankIndex < stationNum; ++rankIndex)
    {
        const unsigned int source = ranks[rankIndex].station;
        if ((stationBitmap & (1 << source)) == 0)
            continue;

        for (unsigned int otherIndex = 0; otherIndex < rankIndex; ++otherIndex)
        {
            const unsigned int destination = ranks[otherIndex].station;
            if ((stationBitmap & (1 << destination)) == 0 ||
                m_routeTable[source * stationNum + destination] != m_invalidRoute)
                continue;

            unsigned int failureReason = cRelayFailureReasonNoRtt;
            bool relayFound = false;
            for (unsigned int candidateIndex = 0, candidateCount = stationNum;
                 candidateCount != 0; --candidateCount, ++candidateIndex)
            {
                const unsigned int relay = relayCandidates[candidateIndex].station;
                const unsigned int relayFlag = 1 << relay;
                if (relay == source || relay == destination ||
                    (stationBitmap & relayFlag) == 0 || (*refusedStationBitmap & relayFlag) != 0)
                    continue;

                RelayFailureReason reason;
                if (IsRelayEnabled(source, relay, destination, &reason))
                {
                    m_routeTable[source * stationNum + destination] =
                        static_cast<unsigned char>(relay);
                    m_routeTable[destination * stationNum + source] =
                        static_cast<unsigned char>(relay);
                    for (unsigned int index = 0, indexCount = stationNum; indexCount != 0;
                         --indexCount, ++index)
                    {
                        if (relayCandidates[index].station == source ||
                            relayCandidates[index].station == destination)
                            relayCandidates[index].brokenRouteCount -= 2;
                    }
                    sortStationRanks(relayCandidates, stationNum);
                    m_relayNumTable[relay] += 2;
                    relayFound = true;
                    break;
                }
                if (reason != cRelayFailureReasonNoRtt)
                    failureReason = reason;
            }

            if (relayFound ||
                CalcRelayRouteWithSwitchingRelay(source, destination,
                                                 stationBitmap & ~*refusedStationBitmap))
                continue;

            unsigned int refusedStation = 0;
            if (source == m_unknown24)
            {
                refusedStation = destination;
            }
            else if (destination == m_unknown24)
            {
                refusedStation = source;
            }
            else
            {
                for (unsigned int index = 0, indexCount = stationNum; indexCount != 0;
                     --indexCount, ++index)
                {
                    if (relayCandidates[index].station == source)
                    {
                        refusedStation = destination;
                        break;
                    }
                    if (relayCandidates[index].station == destination)
                    {
                        refusedStation = source;
                        break;
                    }
                }
            }

            *refusedStationBitmap |= 1 << refusedStation;
            m_failureReasonTable[refusedStation] = static_cast<unsigned char>(failureReason);
            for (unsigned int other = 0, otherCount = stationNum; otherCount != 0;
                 --otherCount, ++other)
            {
                unsigned int first = refusedStation;
                unsigned int second = other;
                for (unsigned int direction = 0; direction < 2; ++direction)
                {
                    const unsigned int relay = m_routeTable[first * stationNum + second];
                    if (relay != first && relay != second && relay < stationNum)
                        --m_relayNumTable[relay];
                    m_routeTable[first * stationNum + second] = static_cast<unsigned char>(first);
                    first = other;
                    second = refusedStation;
                }
            }

            for (unsigned int row = 0, rowCount = stationNum; rowCount != 0; --rowCount, ++row)
            {
                for (unsigned int column = 0, columnCount = stationNum; columnCount != 0;
                     --columnCount, ++column)
                {
                    const unsigned int relay = m_routeTable[row * stationNum + column];
                    if (relay != row && relay != column && relay == refusedStation)
                        m_routeTable[row * stationNum + column] = m_invalidRoute;
                }
            }
            return false;
        }
    }
    return true;
}

void RelayRouteManager::ProcStayStation(unsigned int stayStationBitmap,
                                        unsigned int* refusedStationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
         --sourceCount, ++source)
    {
        if ((stayStationBitmap & (1 << source)) == 0)
            continue;

        for (unsigned int destination = 0, destinationCount = stationNum; destinationCount != 0;
             --destinationCount, ++destination)
        {
            if (source == destination || (stayStationBitmap & (1 << destination)) == 0 ||
                m_rttTable[source * stationNum + destination] != 0)
                continue;

            const unsigned int relay = m_routeTable[source * stationNum + destination];
            if (relay == source)
                continue;

            if (relay != destination)
            {
                if (relay >= stationNum)
                    continue;
                if (m_rttTable[source * stationNum + relay] != 0 &&
                    m_rttTable[relay * stationNum + destination] != 0)
                    continue;
            }
            m_routeTable[source * stationNum + destination] = m_invalidRoute;
        }
    }

    CalcRelayNumTable();
    while (!DecideBrokenRouteRelayStation(stayStationBitmap, refusedStationBitmap))
    {
    }
}

void RelayRouteManager::Trace(unsigned long long) const
{
}

bool RelayRouteManager::ProcNewStationOne(unsigned int station, unsigned int stationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int destination = 0; destination < stationNum; ++destination)
    {
        if ((stationBitmap & (1 << destination)) == 0)
            continue;

        if (m_rttTable[station * stationNum + destination] != 0)
        {
            m_routeTable[station * stationNum + destination] =
                static_cast<unsigned char>(destination);
            m_routeTable[destination * stationNum + station] = static_cast<unsigned char>(station);
            continue;
        }

        unsigned int failureReason = cRelayFailureReasonNoRtt;
        bool relayFound = false;
        for (unsigned int relay = 0, relayCount = stationNum; relayCount != 0;
             --relayCount, ++relay)
        {
            if ((stationBitmap & (1 << relay)) != 0 && relay != station && relay != destination)
            {
                RelayFailureReason reason;
                if (IsRelayEnabled(station, relay, destination, &reason))
                {
                    m_routeTable[station * stationNum + destination] =
                        static_cast<unsigned char>(relay);
                    m_routeTable[destination * stationNum + station] =
                        static_cast<unsigned char>(relay);
                    m_relayNumTable[relay] += 2;
                    relayFound = true;
                    break;
                }
                if (reason != cRelayFailureReasonNoRtt)
                    failureReason = reason;
            }
        }

        if (relayFound || CalcRelayRouteWithSwitchingRelay(station, destination, stationBitmap))
            continue;

        m_failureReasonTable[station] = static_cast<unsigned char>(failureReason);
        for (unsigned int other = 0, otherCount = destination; otherCount != 0;
             --otherCount, ++other)
        {
            const unsigned int relay = m_routeTable[station * stationNum + other];
            if (relay != station && relay != other)
                m_relayNumTable[relay] -= 2;
            m_routeTable[station * stationNum + other] = static_cast<unsigned char>(station);
            m_routeTable[other * stationNum + station] = static_cast<unsigned char>(other);
        }
        return false;
    }
    return true;
}

void RelayRouteManager::ProcNewStation(unsigned int newStationBitmap,
                                       unsigned int* refusedStationBitmap,
                                       unsigned int* acceptedStationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    if ((newStationBitmap & (1 << m_unknown24)) != 0)
    {
        ProcNewStationOne(m_unknown24, *acceptedStationBitmap);
        const unsigned int stationFlag = 1 << m_unknown24;
        newStationBitmap -= stationFlag;
        *acceptedStationBitmap |= stationFlag;
    }

    bool accepted;
    do
    {
        accepted = false;
        for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
             --stationCount, ++station)
        {
            const unsigned int stationFlag = 1 << station;
            if ((newStationBitmap & stationFlag) != 0 &&
                ProcNewStationOne(station, *acceptedStationBitmap))
            {
                accepted = true;
                newStationBitmap -= stationFlag;
                *acceptedStationBitmap |= stationFlag;
            }
        }
    } while (accepted);

    for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
         --stationCount, ++station)
    {
        const unsigned int stationFlag = 1 << station;
        if ((newStationBitmap & stationFlag) != 0)
            *refusedStationBitmap |= stationFlag;
    }
}

bool RelayRouteManager::RelayRouteOptimizationRTTOne(unsigned int stationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int longestRtt = 0;
    unsigned int longestSource = 0;
    unsigned int longestDestination = 0;
    unsigned int longestRelay = 0;
    for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
         --sourceCount, ++source)
    {
        if ((stationBitmap & (1 << source)) == 0)
            continue;

        for (unsigned int destination = source + 1, destinationCount = stationNum - destination;
             destinationCount != 0; --destinationCount, ++destination)
        {
            if ((stationBitmap & (1 << destination)) == 0)
                continue;

            const unsigned int relay = m_routeTable[source * stationNum + destination];
            if (relay == source || relay == destination)
                continue;

            const unsigned int rtt = m_rttTable[source * stationNum + relay] +
                                     m_rttTable[relay * stationNum + destination];
            if (longestRtt < rtt)
            {
                longestSource = source;
                longestDestination = destination;
                longestRtt = rtt;
                longestRelay = relay;
            }
        }
    }

    if (longestRtt == 0)
        return false;

    RttCandidate candidates[32];
    for (unsigned int index = 0, indexCount = 32; indexCount != 0; --indexCount, ++index)
    {
        candidates[index].station = 0;
        candidates[index].rtt = 0;
    }
    unsigned int candidateNum = 0;
    for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
         --stationCount, ++station)
    {
        if ((stationBitmap & (1 << station)) == 0)
            continue;

        const unsigned int sourceRtt = m_rttTable[longestSource * stationNum + station];
        if (sourceRtt == 0)
            continue;

        const unsigned int destinationRtt = m_rttTable[station * stationNum + longestDestination];
        if (destinationRtt == 0)
            continue;

        const unsigned int rtt = sourceRtt + destinationRtt;
        if (rtt < longestRtt && rtt <= m_rttMax)
        {
            candidates[candidateNum].station = station;
            candidates[candidateNum].rtt = rtt;
            ++candidateNum;
        }
    }

    if (candidateNum == 0)
        return false;

    for (unsigned int pass = candidateNum - 1; pass != 0; --pass)
    {
        for (unsigned int index = 0, indexCount = pass; indexCount != 0; --indexCount, ++index)
        {
            if (candidates[index + 1].rtt < candidates[index].rtt)
            {
                const RttCandidate swapped = candidates[index];
                candidates[index] = candidates[index + 1];
                candidates[index + 1] = swapped;
            }
        }
    }

    for (unsigned int index = 0, indexCount = candidateNum; indexCount != 0; --indexCount, ++index)
    {
        const unsigned int relay = candidates[index].station;
        bool switched = m_relayNumTable[relay] + 2 <= m_relayNumMax;
        if (!switched && m_relayNumTable[relay] <= m_relayNumMax)
        {
            m_relayNumTable[longestRelay] -= 2;
            switched = SwitchRelay(relay, stationBitmap,
                                   static_cast<unsigned short>(longestRtt - 1), m_relayNumMax);
            m_relayNumTable[longestRelay] += 2;
        }

        if (switched)
        {
            m_routeTable[longestSource * stationNum + longestDestination] =
                static_cast<unsigned char>(relay);
            m_routeTable[longestDestination * stationNum + longestSource] =
                static_cast<unsigned char>(relay);
            m_relayNumTable[longestRelay] -= 2;
            m_relayNumTable[relay] += 2;
            return true;
        }
    }
    return false;
}

void RelayRouteManager::RelayRouteOptimizationRTT(unsigned int stationBitmap)
{
    while (RelayRouteOptimizationRTTOne(stationBitmap))
    {
    }
}

bool RelayRouteManager::RelayRouteOptimizationRelayNumOne(unsigned int stationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int longestRtt = 0;
    unsigned int busiestStation;
    for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
         --sourceCount, ++source)
    {
        if ((stationBitmap & (1 << source)) == 0)
            continue;

        for (unsigned int destination = source + 1, destinationCount = stationNum - destination;
             destinationCount != 0; --destinationCount, ++destination)
        {
            if ((stationBitmap & (1 << destination)) == 0)
                continue;

            const unsigned int relay = m_routeTable[source * stationNum + destination];
            if (relay != source && relay != destination)
            {
                const unsigned int rtt = m_rttTable[source * stationNum + relay] +
                                         m_rttTable[relay * stationNum + destination];
                if (longestRtt < rtt)
                    longestRtt = rtt;
            }
        }
    }

    unsigned short busiestRelayNum = 0;
    if (stationNum != 0)
    {
        for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
             --stationCount, ++station)
        {
            if (busiestRelayNum < m_relayNumTable[station])
            {
                busiestStation = station;
                busiestRelayNum = m_relayNumTable[station];
            }
        }

        if (busiestRelayNum != 0 &&
            SwitchRelay(busiestStation, stationBitmap, static_cast<unsigned short>(longestRtt),
                        busiestRelayNum - 1))
            return true;
    }
    return false;
}

void RelayRouteManager::RelayRouteOptimizationRelayNum(unsigned int stationBitmap)
{
    while (RelayRouteOptimizationRelayNumOne(stationBitmap))
    {
    }
}

void RelayRouteManager::RelayRouteOptimization(unsigned int stationBitmap)
{
    RelayRouteOptimizationRTT(stationBitmap);
    RelayRouteOptimizationRelayNum(stationBitmap);
}

void RelayRouteManager::ProcRefusedStation(unsigned int refusedStationBitmap,
                                           unsigned int acceptedStationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int refused = 0, refusedCount = stationNum; refusedCount != 0;
         --refusedCount, ++refused)
    {
        if ((refusedStationBitmap & (1 << refused)) == 0)
            continue;

        for (unsigned int destination = 0, destinationCount = stationNum; destinationCount != 0;
             --destinationCount, ++destination)
        {
            if (((acceptedStationBitmap | refusedStationBitmap) & (1 << destination)) == 0 ||
                refused == destination)
                continue;

            if (m_rttTable[refused * stationNum + destination] != 0)
            {
                m_routeTable[refused * stationNum + destination] =
                    static_cast<unsigned char>(destination);
                m_routeTable[destination * stationNum + refused] =
                    static_cast<unsigned char>(refused);
                continue;
            }

            bool relayFound = false;
            for (unsigned int relay = 0, relayCount = stationNum; relayCount != 0;
                 --relayCount, ++relay)
            {
                if ((acceptedStationBitmap & (1 << relay)) != 0 &&
                    IsRelayEnabled(refused, relay, destination, 0))
                {
                    m_routeTable[refused * stationNum + destination] =
                        static_cast<unsigned char>(relay);
                    m_routeTable[destination * stationNum + refused] =
                        static_cast<unsigned char>(relay);
                    m_relayNumTable[relay] += 2;
                    relayFound = true;
                    break;
                }
            }
            if (relayFound)
                continue;

            for (unsigned int relay = 0, relayCount = stationNum; relayCount != 0;
                 --relayCount, ++relay)
            {
                if ((acceptedStationBitmap & (1 << relay)) != 0 &&
                    m_rttTable[refused * stationNum + relay] != 0 &&
                    m_rttTable[relay * stationNum + destination] != 0)
                {
                    m_routeTable[refused * stationNum + destination] =
                        static_cast<unsigned char>(relay);
                    m_routeTable[destination * stationNum + refused] =
                        static_cast<unsigned char>(relay);
                    m_relayNumTable[relay] += 2;
                    break;
                }
            }
        }
    }
}

void RelayRouteManager::UpdateJoiningStationBitmap()
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int joiningStationBitmap = 0;
    m_joiningStationBitmap = 0;
    for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
         --stationCount, ++station)
    {
        for (unsigned int other = 0, otherCount = stationNum; otherCount != 0;
             --otherCount, ++other)
        {
            if (m_routeTable[station * stationNum + other] != station)
            {
                joiningStationBitmap |= 1 << station;
                m_joiningStationBitmap = joiningStationBitmap;
                break;
            }
        }
    }
}

Result RelayRouteManager::UpdateDirectConnectionData(const unsigned char* rttBuffer,
                                                     unsigned int rttBufferSize,
                                                     unsigned int stationBitmap,
                                                     unsigned int* refusedStationBitmap,
                                                     ::nn::pia::StationIndex stationIndex,
                                                     unsigned char* failureReasons)
{
    m_unknown24 = stationIndex;
    if (m_joiningStationBitmap == 0 && (stationBitmap & (1 << stationIndex)) == 0)
        m_joiningStationBitmap = 1 << stationIndex;

    const unsigned int stationNum = g_piaTransport->m_stationNum;
    const unsigned int tableSize = stationNum * stationNum;
    if (rttBufferSize != tableSize)
        return 0x10c07;
    if (rttBuffer == 0)
        return 0x10c07;
    if (refusedStationBitmap == 0)
        return 0x10c07;
    if (failureReasons == 0)
        return 0x10c07;

    for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
         --sourceCount, ++source)
    {
        for (unsigned int destination = 0, destinationCount = stationNum; destinationCount != 0;
             --destinationCount, ++destination)
            m_rttTable[source * stationNum + destination] =
                static_cast<unsigned short>(rttBuffer[source * stationNum + destination] << 2);
    }

    for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
         --sourceCount, ++source)
    {
        unsigned int directStationBitmap = 0;
        for (unsigned int destination = 0, destinationCount = stationNum; destinationCount != 0;
             --destinationCount, ++destination)
        {
            if (m_rttTable[source * stationNum + destination] != 0)
                directStationBitmap |= 1 << destination;
        }
        m_directStationBitmaps[source] = directStationBitmap;
    }

    if (m_relayEmulationEnabled)
    {
        for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
             --sourceCount, ++source)
        {
            for (unsigned int destination = 0, destinationCount = stationNum; destinationCount != 0;
                 --destinationCount, ++destination)
            {
                if (m_relayEmulationTable[source * stationNum + destination] == 1)
                    m_rttTable[source * stationNum + destination] = 0;
            }
        }
    }

    for (unsigned int source = 1; source < stationNum; ++source)
    {
        for (unsigned int destination = 0; destination < source; ++destination)
        {
            if (m_rttTable[source * stationNum + destination] == 0 ||
                m_rttTable[destination * stationNum + source] == 0)
            {
                m_rttTable[source * stationNum + destination] = 0;
                m_rttTable[destination * stationNum + source] = 0;
            }
            else
            {
                const unsigned short averageRtt = static_cast<unsigned short>(
                    (m_rttTable[source * stationNum + destination] +
                     m_rttTable[destination * stationNum + source]) >> 1);
                m_rttTable[source * stationNum + destination] = averageRtt;
                m_rttTable[destination * stationNum + source] = averageRtt;
            }
        }
    }

    *refusedStationBitmap = 0;
    unsigned int leaveStationBitmap = 0;
    unsigned int stayStationBitmap = 0;
    CalcStationType(stationBitmap, &stayStationBitmap, &leaveStationBitmap);
    for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
         --stationCount, ++station)
    {
        const unsigned int stationFlag = 1 << station;
        if ((stayStationBitmap & stationFlag) != 0 && (m_unknown30 & stationFlag) != 0)
        {
            stayStationBitmap &= ~stationFlag;
            leaveStationBitmap |= stationFlag;
            *refusedStationBitmap |= stationFlag;
        }
    }

    ProcLeaveStation(leaveStationBitmap);
    ProcStayStation(stayStationBitmap, refusedStationBitmap);

    const unsigned int refusedByStay = *refusedStationBitmap;
    *refusedStationBitmap = 0;
    unsigned int acceptedStationBitmap = stayStationBitmap & ~refusedByStay;
    ProcNewStation(stationBitmap | refusedByStay, refusedStationBitmap, &acceptedStationBitmap);
    for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
         --stationCount, ++station)
    {
        unsigned char failureReason = 0;
        if ((*refusedStationBitmap & (1 << station)) != 0)
            failureReason = m_failureReasonTable[station];
        failureReasons[station] = failureReason;
    }

    RelayRouteOptimization(acceptedStationBitmap);
    ProcRefusedStation(*refusedStationBitmap, acceptedStationBitmap);
    UpdateJoiningStationBitmap();
    m_unknown30 = *refusedStationBitmap;
    std::memcpy(m_originalRouteTable, m_routeTable, tableSize);
    return 0;
}

Result RelayRouteManager::WriteRelayRouteDirectionsToBuffer(unsigned char* buffer,
                                                            unsigned int size,
                                                            unsigned int* writtenSize) const
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int routeBits = m_routeBitCount * stationNum * stationNum;
    unsigned int bitsUsed = 0;
    unsigned int offset = 0;
    if ((routeBits & 7) != 0)
        routeBits = (routeBits & ~7u) + 8;
    const unsigned int routeBytes = routeBits >> 3;
    if (size < routeBytes + stationNum * 4 + 4)
    {
        *writtenSize = 0;
        return 0x10c07;
    }
    if (buffer == 0)
        return 0x10c07;

    std::memset(buffer, 0, size);
    for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
         --sourceCount, ++source)
    {
        for (unsigned int destination = 0, destinationCount = stationNum;
             destinationCount != 0; --destinationCount, ++destination)
        {
            const char freeBits = static_cast<char>(8 - bitsUsed - m_routeBitCount);
            unsigned int shift = freeBits;
            const unsigned char route = m_routeTable[source * stationNum + destination];
            if (freeBits < 0)
            {
                const unsigned int fitBits = m_routeBitCount + freeBits;
                bitsUsed = -shift;
                shift = static_cast<char>(freeBits + 8);
                buffer[offset] += (route >> bitsUsed) & ((1 << fitBits) - 1);
                ++offset;
            }
            bitsUsed = (8 - shift) & 0xff;
            buffer[offset] += route << shift;
            if (shift == 0)
            {
                ++offset;
                bitsUsed = 0;
            }
        }
    }

    offset = routeBytes;
    for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
         --stationCount, ++station)
    {
        unsigned int directStationBitmap = m_directStationBitmaps[station];
        for (unsigned int byteCount = 4, shift = 24; byteCount != 0; --byteCount, shift -= 8)
        {
            const unsigned int value = directStationBitmap >> shift;
            buffer[offset] = static_cast<unsigned char>(value);
            ++offset;
            directStationBitmap -= value << shift;
        }
    }

    std::memcpy(buffer + offset, &m_unknown30, 4);
    *writtenSize = offset + 4;
    return 0;
}

Result RelayRouteManager::ReadRelayRouteDirectionsFromBuffer(const unsigned char* buffer,
                                                             unsigned int size)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int routeBits = m_routeBitCount * stationNum * stationNum;
    unsigned int bitsUsed = 0;
    unsigned int offset = 0;
    if ((routeBits & 7) != 0)
        routeBits = (routeBits & ~7u) + 8;
    const unsigned int routeBytes = routeBits >> 3;
    if (size < routeBytes + stationNum * 4 + 4)
        return 0x10c07;
    if (buffer == 0)
        return 0x10c07;

    for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
         --sourceCount, ++source)
    {
        for (unsigned int destination = 0, destinationCount = stationNum;
             destinationCount != 0; --destinationCount, ++destination)
        {
            const char freeBits = static_cast<char>(8 - bitsUsed - m_routeBitCount);
            unsigned int shift = freeBits;
            unsigned char highBits = 0;
            if (freeBits < 0)
            {
                const unsigned int fitBits = m_routeBitCount + freeBits;
                const unsigned int highShift = -shift;
                shift = static_cast<char>(freeBits + 8);
                highBits = static_cast<unsigned char>(
                    (buffer[offset] & ((1 << fitBits) - 1)) << highShift);
                ++offset;
                bitsUsed = 0;
            }
            const unsigned int lowBits = (8 - shift) - bitsUsed;
            bitsUsed = (8 - shift) & 0xff;
            m_routeTable[source * stationNum + destination] =
                highBits + ((buffer[offset] >> shift) & ((1 << lowBits) - 1));
            if (shift == 0)
            {
                ++offset;
                bitsUsed = 0;
            }
        }
    }

    offset = routeBytes;
    for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
         --stationCount, ++station)
    {
        unsigned int directStationBitmap = 0;
        for (unsigned int byteCount = 4, shift = 24; byteCount != 0; --byteCount, shift -= 8)
        {
            directStationBitmap += buffer[offset] << shift;
            ++offset;
        }
        m_directStationBitmaps[station] = directStationBitmap;
    }

    std::memcpy(&m_unknown30, buffer + offset, 4);
    UpdateJoiningStationBitmap();
    std::memcpy(m_originalRouteTable, m_routeTable, stationNum * stationNum);
    return 0;
}

unsigned int RelayRouteManager::GetRelayRouteDirectionsSize() const
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int routeBits = m_routeBitCount * stationNum * stationNum;
    if ((routeBits & 7) != 0)
        routeBits = (routeBits & ~7u) + 8;
    return (routeBits >> 3) + stationNum * 4 + 4;
}

Result RelayRouteManager::GetRelayRoute(::nn::pia::StationIndex source,
                                        ::nn::pia::StationIndex destination,
                                        ::nn::pia::StationIndex* relay) const
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    if (static_cast<unsigned int>(source) >= stationNum ||
        static_cast<unsigned int>(destination) >= stationNum)
        return 0x10c07;
    if (relay == 0)
        return 0x10c07;
    *relay = static_cast< ::nn::pia::StationIndex>(
        m_routeTable[source * stationNum + destination]);
    return 0;
}

Result RelayRouteManager::SetRelayRoute(::nn::pia::StationIndex source,
                                        ::nn::pia::StationIndex destination,
                                        ::nn::pia::StationIndex relay)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    if (static_cast<unsigned int>(source) >= stationNum ||
        static_cast<unsigned int>(destination) >= stationNum)
        return 0x10c07;
    m_routeTable[source * stationNum + destination] = static_cast<unsigned char>(relay);
    return 0;
}

Result RelayRouteManager::GetDestStationList(::nn::pia::StationIndex source,
                                             ::nn::pia::StationIndex relay,
                                             unsigned int* destinationBitmap) const
{
    unsigned int stationNum = g_piaTransport->m_stationNum;
    if (static_cast<unsigned int>(source) >= stationNum ||
        static_cast<unsigned int>(relay) >= stationNum)
        return 0x10c07;
    if (destinationBitmap == 0)
        return 0x10c07;

    unsigned int bitmap = 0;
    unsigned int stationFlag = 1;
    *destinationBitmap = 0;
    unsigned int index = source * stationNum;
    while (stationNum != 0)
    {
        if (m_routeTable[index] == relay)
        {
            bitmap |= stationFlag;
            *destinationBitmap = bitmap;
        }
        stationFlag <<= 1;
        ++index;
        --stationNum;
    }
    return 0;
}

Result RelayRouteManager::GetDirectStationList(::nn::pia::StationIndex source,
                                               unsigned int* directBitmap) const
{
    unsigned int stationNum = g_piaTransport->m_stationNum;
    if (static_cast<unsigned int>(source) >= stationNum)
        return 0x10c07;
    if (directBitmap == 0)
        return 0x10c07;

    unsigned int bitmap = 0;
    unsigned int stationFlag = 1;
    *directBitmap = 0;
    unsigned int index = source * stationNum;
    unsigned int destination = 0;
    while (stationNum != 0)
    {
        if (m_routeTable[index] == destination)
        {
            bitmap |= stationFlag;
            *directBitmap = bitmap;
        }
        stationFlag <<= 1;
        ++index;
        ++destination;
        --stationNum;
    }
    return 0;
}

void RelayRouteManager::SetRttMax(unsigned short rttMax)
{
    m_rttMax = rttMax;
    g_rttMax = rttMax;
}

void RelayRouteManager::SetRelayNumMax(unsigned short relayNumMax)
{
    m_relayNumMax = relayNumMax & 0xfffe;
    g_relayNumMax = relayNumMax & 0xfffe;
}

void RelayRouteManager::UpdateRelayTable(unsigned int stationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int station = 0, stationCount = stationNum; stationCount != 0;
         --stationCount, ++station)
    {
        const unsigned int stationFlag = 1 << station;
        unsigned int joiningStationBitmap = m_joiningStationBitmap;
        if ((joiningStationBitmap & stationFlag) == 0 && (stationBitmap & stationFlag) != 0)
        {
            for (unsigned int other = 0, otherCount = stationNum; otherCount != 0;
                 --otherCount, ++other)
            {
                if (((joiningStationBitmap | stationBitmap) & (1 << other)) != 0)
                {
                    m_routeTable[station * stationNum + other] = static_cast<unsigned char>(other);
                    m_routeTable[other * stationNum + station] =
                        static_cast<unsigned char>(station);
                    joiningStationBitmap = m_joiningStationBitmap;
                }
            }
            m_joiningStationBitmap = joiningStationBitmap | stationFlag;
        }
    }

    for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
         --sourceCount, ++source)
    {
        for (unsigned int destination = source, destinationCount = stationNum - source;
             destinationCount != 0; --destinationCount, ++destination)
        {
            if (((m_joiningStationBitmap & (1 << source)) == 0 ||
                 (m_joiningStationBitmap & (1 << destination)) == 0) &&
                (m_routeTable[source * stationNum + destination] != source ||
                 m_routeTable[destination * stationNum + source] != destination))
            {
                m_routeTable[source * stationNum + destination] =
                    static_cast<unsigned char>(source);
                m_routeTable[destination * stationNum + source] =
                    static_cast<unsigned char>(destination);
            }
        }
    }
}

Result RelayRouteManager::SearchRefugeRelayRoute(::nn::pia::StationIndex station)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    CalcRelayNumTable();
    if (m_relayNumTable[station] == 0)
        return 0;

    bool found = true;
    if (stationNum != 0)
    {
        for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
             --sourceCount, ++source)
        {
            if (source == station)
                continue;

            for (unsigned int destination = source + 1, destinationCount = stationNum - destination;
                 destinationCount != 0; --destinationCount, ++destination)
            {
                if (destination == station ||
                    m_routeTable[source * stationNum + destination] != station)
                    continue;

                unsigned int bestRelay = stationNum;
                unsigned int bestRelayNum = stationNum * stationNum;
                for (unsigned int relay = 0, relayCount = stationNum; relayCount != 0;
                     --relayCount, ++relay)
                {
                    if (relay != source && relay != destination && relay != station &&
                        m_routeTable[source * stationNum + relay] == relay &&
                        m_routeTable[relay * stationNum + destination] == destination &&
                        m_relayNumTable[relay] < bestRelayNum)
                    {
                        bestRelayNum = m_relayNumTable[relay];
                        bestRelay = relay;
                    }
                }

                if (bestRelay == stationNum)
                {
                    found = false;
                    continue;
                }

                m_routeTable[source * stationNum + destination] =
                    static_cast<unsigned char>(bestRelay);
                m_routeTable[destination * stationNum + source] =
                    static_cast<unsigned char>(bestRelay);
                m_relayNumTable[station] -= 2;
                m_relayNumTable[bestRelay] += 2;
            }
        }

        if (!found)
            return 0x2c0a;
    }
    return 0;
}

void RelayRouteManager::InitRelayRouteManagerData()
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int source = 0, sourceCount = stationNum; sourceCount != 0;
         --sourceCount, ++source)
    {
        for (unsigned int destination = 0, destinationCount = stationNum; destinationCount != 0;
             --destinationCount, ++destination)
            m_routeTable[source * stationNum + destination] = static_cast<unsigned char>(source);
    }
    m_joiningStationBitmap = 0;
    m_unknown30 = 0;
    m_unknown20 = 0;
    m_unknown1c = 0;
}

Result RelayRouteManager::GetOriginalRelayRoute(::nn::pia::StationIndex source,
                                                ::nn::pia::StationIndex destination,
                                                ::nn::pia::StationIndex* relay) const
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    if (static_cast<unsigned int>(source) >= stationNum ||
        static_cast<unsigned int>(destination) >= stationNum)
        return 0x10c07;
    if (relay == 0)
        return 0x10c07;
    *relay = static_cast< ::nn::pia::StationIndex>(
        m_originalRouteTable[source * stationNum + destination]);
    return 0;
}

Result RelayRouteManager::GetOriginalDirectStationList(::nn::pia::StationIndex source,
                                                       unsigned int* directBitmap) const
{
    unsigned int stationNum = g_piaTransport->m_stationNum;
    if (static_cast<unsigned int>(source) >= stationNum)
        return 0x10c07;
    if (directBitmap == 0)
        return 0x10c07;

    unsigned int bitmap = 0;
    unsigned int stationFlag = 1;
    *directBitmap = 0;
    unsigned int index = source * stationNum;
    unsigned int destination = 0;
    while (stationNum != 0)
    {
        if (m_originalRouteTable[index] == destination)
        {
            bitmap |= stationFlag;
            *directBitmap = bitmap;
        }
        stationFlag <<= 1;
        ++index;
        ++destination;
        --stationNum;
    }
    return 0;
}

}}}
