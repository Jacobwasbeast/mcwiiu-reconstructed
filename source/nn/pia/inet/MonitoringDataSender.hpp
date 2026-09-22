#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_MONITORING_DATA_SENDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_MONITORING_DATA_SENDER_HPP

#include "nn/pia/common/MonitoringDataSender.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/inet/MonitoringDataSendJob.hpp"
#include "nn/pia/inet/MonitoringServerAddressResolveJob.hpp"

namespace nn { namespace pia { namespace inet {

class MonitoringDataSender : public common::MonitoringDataSender
{
public:
    MonitoringDataSender();
    virtual ~MonitoringDataSender();
    Result Startup();
    void Cleanup();
    void SetSendSessionBeginReportRequested(bool requested);
    unsigned char IsSendSessionBeginReportRequested() const;
    void GetIv(unsigned char* destination, unsigned char* source) const;
    void Trace(unsigned long long value) const;
    unsigned int IsEnableMonitoring() const;
    bool IsResolutionInProgress() const;
    const common::InetAddress& GetServerAddress() const;
    DataType GetDataType() const;
    const unsigned char* GetCryptoKey(unsigned char index) const;
    Result MakeReportData(DataType dataType, bool stateOnly);
    void SendReport();
    virtual void RequestSendReportImpl(DataType dataType);
    unsigned int IsPreparedForCleanup() const;

private:
    bool m_sendSessionBeginReportRequested;
};

}}}

#endif
