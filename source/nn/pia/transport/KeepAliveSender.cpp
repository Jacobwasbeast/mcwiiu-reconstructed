#include "nn/pia/transport/KeepAliveSender.hpp"

#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn { namespace pia { namespace transport {

namespace {

struct TickWords
{
    unsigned int high;
    unsigned int low;
};

TickWords multiplyWords(unsigned int left, unsigned int right)
{
    const unsigned long long product =
        static_cast<unsigned long long>(left) * static_cast<unsigned long long>(right);
    TickWords result;
    result.high = static_cast<unsigned int>(product >> 32);
    result.low = static_cast<unsigned int>(product);
    return result;
}

}

KeepAliveSender::KeepAliveSender() : m_interval(1000), m_enabled(true)
{
}

KeepAliveSender::~KeepAliveSender()
{
    if (this == 0)
        return;
}

Result KeepAliveSender::SetInterval(int interval)
{
    if (interval < 0)
    {
        return static_cast<Result>(0x10c07);
    }
    m_interval = static_cast<unsigned int>(interval);
    return 0;
}

unsigned int KeepAliveSender::Update(unsigned int stationBitmap, const common::Time& time)
{
    if (!m_enabled || g_piaStationManager == 0 || g_piaStationManager->m_localStation == 0)
        return 0;

    const TickWords ticks = {static_cast<unsigned int>(common::TimeSpan::GetTicksPerMSec().m_tick >> 32),
                             static_cast<unsigned int>(common::TimeSpan::GetTicksPerMSec().m_tick)};
    const TickWords lowProduct = multiplyWords(ticks.low, m_interval);
    const TickWords highProduct = multiplyWords(ticks.high, m_interval);
    const unsigned int productHigh = highProduct.low + lowProduct.high +
                                     (static_cast<unsigned int>(m_interval) >> 31) * ticks.low;
    const unsigned int timeHigh = static_cast<unsigned int>(time.m_tick >> 32);
    const unsigned int timeLow = static_cast<unsigned int>(time.m_tick);
    const unsigned int thresholdLow = timeLow - lowProduct.low;
    const unsigned int thresholdHigh =
        timeHigh - productHigh - (timeLow < lowProduct.low ? 1u : 0u);
    unsigned int expired = 0;
    const common::SimpleContainer<Station*, StationManager::cStationNumMax>& stations =
        g_piaStationManager->m_stations;
    for (unsigned int index = 0; index < stations.size; ++index)
    {
        Station* station = stations.data[index];
        if (station == 0 || station == g_piaStationManager->m_localStation || station->m_field20 != 5 ||
            static_cast<unsigned int>(station->m_field10) >= 32)
            continue;
        const unsigned int bit = 1u << station->m_field10;
        if ((stationBitmap & bit) == 0)
        {
            if (station->m_field58 <= static_cast<int>(thresholdHigh) &&
                (station->m_field58 < static_cast<int>(thresholdHigh) ||
                 static_cast<unsigned int>(station->m_field5c) < thresholdLow))
                expired |= bit;
            continue;
        }
        station->m_field58 = static_cast<int>(timeHigh);
        station->m_field5c = static_cast<int>(timeLow);
    }
    return expired;
}

}}} 
