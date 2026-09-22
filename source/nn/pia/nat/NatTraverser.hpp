#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSER_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/InetAddress.hpp"

namespace nn { namespace pia {
namespace common { class CallContext; }
namespace inet { class IFacade; }
namespace transport { class StationLocation; class StationConnectionInfo; }
namespace nat {
class NatTraversalProtocol;

class NatTraverser
{
public:
    NatTraverser();
    virtual ~NatTraverser();
    void CompleteNatDetectionCommon();
    void CompletedNatPropertyDetection(bool success);
    void StartNatPropertyDetection();
    void CompletedNatPortDetection(bool success);
    void StartNatPortDetection();
    common::Result Startup(inet::IFacade* facade, common::CallContext* context);
    void Cleanup();
    int GetNatMonitoringDataIndex(const transport::StationLocation& location);
    void IncrementNatTraversalSuccessCount(unsigned int index);
    void IncrementNatTraversalFailureCount(unsigned int index);
    void ReportNatTraversalResult(const transport::StationLocation& location, common::Result result, bool relay);
    unsigned int PrepareNatTraversal(const transport::StationLocation& location,
                                     const transport::StationConnectionInfo& connection,
                                     common::CallContext* context, bool relay, bool passive, bool force);
    void ClearNatTraversal(const transport::StationLocation& location, bool relay);
    unsigned int CheckLatestStationLocation(transport::StationLocation& location);
    void Trace(unsigned long long value) const;
    void CreateProtocols();
    void ClearNatMonitoringData(const transport::StationLocation& location);
    bool IsStartupCancelled();
    void RelayProbeRequest(const transport::StationLocation& source,
                           const transport::StationLocation& destination,
                           bool relay, unsigned short port);
    void ProcessProbeRequest(const transport::StationLocation& location);
    bool IsTraversed(const transport::StationLocation& location) const;
    bool IsTraversed(unsigned int index) const;
    void SetLocalPublicLocation(const transport::StationLocation& location);
    void* GetNatPortDetecter();
    unsigned int GetLocalServiceConnectionId() const;
    unsigned short GetRandomPortRangeMin() const;
    unsigned short GetRandomPortRangeMax() const;
    bool IsNatDetectionCompleted() const;
    void ResetProbeStatus(const transport::StationLocation& location, bool relay);
    void UpdateRelayTable();
    unsigned int GetRelayServerAddress() const;
    bool IsProcessingNatDetection() const;

private:
    common::CallContext* m_activeContext;
    unsigned char m_monitoringData[0x14];
    common::InetAddress m_servers[2];
    unsigned char m_detectionActive;
    unsigned char m_reservedState;
    unsigned short m_portDelta;
    unsigned int m_serverCount;
    NatTraversalProtocol* m_protocol;
    void* m_natPortDetecter;
    void* m_natPropertyDetecter;
    common::CallContext m_propertyContext;
    common::CallContext m_portContext;
    unsigned int m_localServiceConnectionId;
    unsigned short m_randomPortRangeMin;
    unsigned short m_randomPortRangeMax;
    unsigned int m_relayServerAddress;
    bool m_startupCancelled;
    inet::IFacade* m_facade;
    unsigned int m_monitoringConnectionIds[15];
    unsigned int m_successCount;
    unsigned int m_failureCount;
};

}}
}
#endif
