#include "nn/pia/inet/NexSessionStartupSetting.hpp"

namespace nn { namespace pia { namespace inet {

NexSessionStartupSetting::NexSessionStartupSetting()
    : m_payloadSize(1240),
      m_playersCountedAsParticipants(true),
      m_field94(0),
      m_hostMigrationEnabled(true),
      m_field99(false),
      m_bandwidthCheckMeasurementTime(-1),
      m_bandwidthCheckMeasurementCount(1000),
      m_bandwidthCheckIpPacketSize(0),
      m_recentPlayRecordEnabled(true)
{
}

}}}
