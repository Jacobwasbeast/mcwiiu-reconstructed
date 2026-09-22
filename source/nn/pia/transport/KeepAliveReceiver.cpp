#include "nn/pia/transport/KeepAliveReceiver.hpp"

#include "nn/pia/transport/StationManager.hpp"

namespace nn { namespace pia { namespace transport {

KeepAliveReceiver::KeepAliveReceiver() : m_enabled(true)
{
}

KeepAliveReceiver::~KeepAliveReceiver()
{
}

void KeepAliveReceiver::Update(unsigned int stationBitmap, const common::Time& time)
{
    if (!m_enabled || stationBitmap == 0)
        return;

    for (unsigned int index = 0; index != 32; ++index)
    {
        if ((stationBitmap & (1u << index)) == 0)
            continue;
        Station* station = g_piaStationManager->GetStation(static_cast<StationIndex>(index));
        if (station != 0)
        {
            station->m_field60 = static_cast<int>(time.m_tick >> 32);
            station->m_field64 = static_cast<int>(time.m_tick);
        }
    }
}

}}}
