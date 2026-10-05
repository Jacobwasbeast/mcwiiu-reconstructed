#include "nn/pia/session/RelayRouteManageJob.hpp"

#include <coreinit/memory.h>
#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/transport/NetworkRttManager.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
enum
{
    cStationIndexMax = 31,
    cStationStateConnected = 5,
    cReportHeaderSize = 0x10,
    cReportGenerationOffset = 4,
    cReportSerialOffset = 8,
    cTraceRelayRouteLevel = 0x1000000,
    cDummyMessageIntervalMsec = 300,
    cRttShift = 2,
    cRttValueMax = 0xff,
    cRttValueMinimum = 1
};

const char* const cStepNameWaitAllDirectConnectionReport =
    "RelayRouteManageJob::WaitAllDirectConnectionReport";
const char* const cStepNameSendRelayRouteDirections =
    "RelayRouteManageJob::SendRelayRouteDirections";

template <typename T>
inline T* allocateHeapArray(unsigned int count)
{
    T* array = static_cast<T*>(
        pead::allocateFromSeadHeap(sizeof(T) * count, common::HeapManager::GetHeap()));
    if (array == 0)
        return 0;
    T* element = array;
    for (unsigned int remaining = count; remaining != 0; --remaining, ++element)
        new (element) T();
    return array;
}

inline unsigned char encodeRtt(unsigned int rtt)
{
    if (rtt == 0)
        return 0;
    const unsigned int value = rtt >> cRttShift;
    if (value == 0)
        return cRttValueMinimum;
    if (value < cRttValueMax)
        return static_cast<unsigned char>(value);
    return cRttValueMax;
}
}

RelayRouteManageJob::RelayRouteManageJob()
    : common::StepSequenceJob(), m_generation(0), m_sequenceNumber(0), m_resendTime()
{
    m_stationNum = transport::g_piaTransport->m_stationNum;
    m_connectionReports = common::NewArray<unsigned char>(m_stationNum * m_stationNum);
    m_reportSerials = allocateHeapArray<unsigned int>(m_stationNum);
    OSBlockSet(m_connectionReports, 0, m_stationNum * m_stationNum);
    OSBlockSet(m_reportSerials, 0, m_stationNum << 2);
    m_stationBitmap = 0;
    m_refusedStationBitmap = 0;
    m_failureReasons = common::NewArray<unsigned char>(m_stationNum);
    OSBlockSet(m_failureReasons, 0, m_stationNum);
}

RelayRouteManageJob::~RelayRouteManageJob()
{
    common::DeleteArray<unsigned char>(m_connectionReports);
    unsigned int* reportSerials = m_reportSerials;
    if (reportSerials != 0)
    {
        pead::getArraySize(reportSerials);
        pead::freeToSeadHeap(reportSerials);
    }
    common::DeleteArray<unsigned char>(m_failureReasons);
}

void RelayRouteManageJob::Trace(unsigned long long) const
{
}

bool RelayRouteManageJob::Startup(StationIndex stationIndex, unsigned int generation,
                                  unsigned int reportSerial)
{
    if (IsRunning() || stationIndex < 0 || stationIndex >= m_stationNum || generation == 0 ||
        generation < m_generation ||
        (generation == m_generation && m_reportSerials[stationIndex] >= reportSerial))
        return false;

    m_resendTime = common::g_piaJobScheduler->m_dispatchTime;
    Reset(true);
    setStep(&RelayRouteManageJob::WaitAllDirectConnectionReport,
            cStepNameWaitAllDirectConnectionReport);
    return true;
}

void RelayRouteManageJob::UpdateConnectionReport(StationIndex stationIndex,
                                                 const unsigned char* report, unsigned int size)
{
    if (stationIndex < 0 || stationIndex >= m_stationNum || size <= cReportHeaderSize)
        return;

    const unsigned int generation =
        common::deserializeU32(report + cReportGenerationOffset);
    const unsigned int reportSerial = common::deserializeU32(report + cReportSerialOffset);
    if (generation == 0 || generation < m_generation)
        return;

    if (m_generation < generation)
    {
        m_generation = generation;
        OSBlockSet(m_connectionReports, 0, m_stationNum * m_stationNum);
        OSBlockSet(m_reportSerials, 0, m_stationNum << 2);
        m_sequenceNumber = 1;
    }
    else if (reportSerial <= m_reportSerials[stationIndex])
    {
        return;
    }

    OSBlockMove(m_connectionReports + stationIndex * m_stationNum, report + cReportHeaderSize,
                size - cReportHeaderSize, FALSE);
    m_reportSerials[stationIndex] = reportSerial;
}

RelayRouteManageJob::ExecuteResult RelayRouteManageJob::WaitAllDirectConnectionReport()
{
    const StationIndex localStation = g_piaMesh->GetLocalStationIndex();
    if (localStation > cStationIndexMax)
        return MakeExecuteResult(cExecuteKindFinished);

    bool allReported = true;
    const unsigned short localIndex = localStation;
    if (m_stationNum != 0)
    {
        for (unsigned short index = 0; index < m_stationNum; ++index)
        {
            if (index == localIndex)
                continue;

            const bool valid = g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(index));
            const unsigned int reportSerial = m_reportSerials[index];
            if (!valid)
            {
                if (reportSerial != 0)
                    allReported = false;
            }
            else if (reportSerial == 0)
            {
                allReported = false;
            }
        }
        if (!allReported)
            return MakeExecuteResult(cExecuteKindNextStep);
    }

    const common::Time now = common::g_piaJobScheduler->m_dispatchTime;
    unsigned short reportIndex = localIndex * m_stationNum;
    if (m_stationNum != 0)
    {
        for (unsigned short index = 0; index < m_stationNum; ++index, ++reportIndex)
        {
            unsigned int rtt = 0;
            transport::Station* station =
                transport::g_piaStationManager->GetStation(static_cast<StationIndex>(index));
            if (station != 0 && station->m_field20 == cStationStateConnected)
            {
                rtt = transport::NetworkRttManager::sInstance->GetAverage(station->m_stationAddress);
                rtt = static_cast<unsigned short>(rtt);
            }

            unsigned char value;
            if (index != localIndex && g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(index)))
            {
                if (rtt == 0)
                {
                    if (!station->IsConnectionRouteRelay())
                    {
                        if (m_resendTime <= now)
                        {
                            g_piaMesh->GetMeshProtocol()->SendDummyMessage(static_cast<StationIndex>(index));
                            m_resendTime.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick +
                                common::TimeSpan::GetTicksPerMSec().m_tick * cDummyMessageIntervalMsec;
                        }
                        allReported = false;
                    }
                    value = 0;
                }
                else if (station->IsConnectionRouteRelay())
                {
                    value = 0;
                }
                else
                {
                    value = encodeRtt(rtt);
                }
            }
            else
            {
                value = encodeRtt(rtt);
            }
            m_connectionReports[reportIndex] = value;
        }
        if (!allReported)
            return MakeExecuteResult(cExecuteKindNextStep);
    }

    setStep(&RelayRouteManageJob::SendRelayRouteDirections, cStepNameSendRelayRouteDirections);
    return MakeExecuteResult(cExecuteKindContinue);
}

RelayRouteManageJob::ExecuteResult RelayRouteManageJob::SendRelayRouteDirections()
{
    const StationIndex localStation = g_piaMesh->GetLocalStationIndex();
    if (localStation > cStationIndexMax)
        return MakeExecuteResult(cExecuteKindFinished);

    Trace(cTraceRelayRouteLevel);
    transport::RelayRouteManager* manager = transport::g_piaTransport->m_relayRouteManager;
    manager->Trace(cTraceRelayRouteLevel);
    const Result result = manager->UpdateDirectConnectionData(
        m_connectionReports, m_stationNum * m_stationNum, m_stationBitmap, &m_refusedStationBitmap,
        localStation, m_failureReasons);
    if (result == 0)
    {
        manager->m_unknown20 = m_generation;
        manager->m_unknown1c = m_sequenceNumber;
        m_stationBitmap = 0;
        manager->Trace(cTraceRelayRouteLevel);
        if (!g_piaMesh->GetMeshProtocol()->SendRelayRouteDirections())
        {
            setStep(&RelayRouteManageJob::WaitAllDirectConnectionReport,
                    cStepNameWaitAllDirectConnectionReport);
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        ++m_sequenceNumber;
    }
    return MakeExecuteResult(cExecuteKindFinished);
}

void RelayRouteManageJob::Cleanup()
{
    m_generation = 0;
    OSBlockSet(m_connectionReports, 0, m_stationNum * m_stationNum);
    OSBlockSet(m_reportSerials, 0, m_stationNum << 2);
    m_refusedStationBitmap = 0;
    m_stationBitmap = 0;
}

void RelayRouteManageJob::PrepareForBecomingNewHost()
{
    transport::g_piaTransport->m_relayRouteManager->UpdateRelayTable(m_stationBitmap);
    m_stationBitmap = 0;
}

}}}
