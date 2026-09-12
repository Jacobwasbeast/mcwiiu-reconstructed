#include "nn/pia/transport/StationManager.hpp"

#include "nn/pia.h"
#include "nn/pia/common/StationAddress.hpp"

namespace nn { namespace pia { namespace transport {

void* StationManager::GetStation(const common::StationAddress& address)
{
    for (unsigned int index = 0; index < m_stationNum; ++index)
    {
        if (m_stations[index]->m_stationAddress == address)
            return m_stations[index];
    }

    return 0;
}

}}}
