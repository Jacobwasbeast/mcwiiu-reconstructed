#include "nn/pia/lan/LanConnectStationJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"

namespace nn { namespace pia { namespace lan {

LanConnectStationJob::LanConnectStationJob()
{
    m_state = 0x01000000;
    m_result = 0;
}

LanConnectStationJob::~LanConnectStationJob()
{
}

int LanConnectStationJob::StartupImpl(common::CallContext* context,
    transport::Station* station, const transport::StationConnectionInfo& info, bool secure)
{
    m_station = station;
    m_secure = secure;
    info.Trace(0x0100000000000000ULL);
    m_connectionInfo = info;
    m_connectionInfo.m_stationLocation.m_field25 = 3;
    m_connectionInfo.m_localStationLocation.m_field25 = 0;
    m_connectionInfo.Trace(0x0100000000000000ULL);
    station->m_stationAddress.Trace(0x0100000000000000ULL);
    session::g_piaMesh->TraceStationConnectionInfoList(0x0200000000000000ULL);
    m_currentLocation.SetStationLocation(m_connectionInfo.m_localStationLocation);
    if (station->m_field10 == 0xfe)
    {
        transport::g_stationConnectionInfoTable->m_hostStationConnectionInfo = m_connectionInfo;
        transport::g_stationConnectionInfoTable->m_hostStationConnectionInfo.Trace(0x0100000000000000ULL);
        transport::g_stationConnectionInfoTable->m_hostStationAddress = m_currentLocation.m_stationAddress;
        transport::g_stationConnectionInfoTable->m_hostStationAddress.Trace(0x0100000000000000ULL);
    }
    transport::StationConnectionInfo currentInfo;
    currentInfo.m_stationLocation.SetStationLocation(m_currentLocation);
    const int result = transport::ConnectStationJob::StartupImpl(context, station, currentInfo, secure);
    if (result != 0)
        return result;
    m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
        &LanConnectStationJob::TryCurrentAddress);
    m_currentStep.m_name = "LanConnectStationJob::TryCurrentAddress";
    return 0;
}

void LanConnectStationJob::Trace(unsigned long long) const
{
}

unsigned int LanConnectStationJob::TryCurrentAddress()
{
    m_station->m_stationAddress = m_currentLocation.m_stationAddress;
    m_station->Trace(0x0200000000000000ULL);
    m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
        &transport::ConnectStationJob::SendConnectionRequest);
    m_currentStep.m_name = "ConnectStationJob::SendConnectionRequest";
    return cExecuteResultContinue;
}

}}}
