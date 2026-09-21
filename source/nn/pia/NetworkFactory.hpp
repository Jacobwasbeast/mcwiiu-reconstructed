#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NETWORKFACTORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NETWORKFACTORY_HPP

namespace nn
{
namespace pia
{
namespace transport
{

class ConnectStationJob;
class DisconnectStationJob;

class NetworkFactory
{
public:
    NetworkFactory();
    virtual ~NetworkFactory();
    virtual unsigned int GetMaxStationNum() const = 0;
    virtual unsigned int GetHeaderSize() const = 0;
    virtual unsigned int GetMtuSizeMax() const = 0;
    virtual unsigned int GetMtuSizeMin() const = 0;
    virtual int GetDefaultRttInitialPulseInterval() const = 0;
    virtual bool IsClientRelayConnectionPermitted() const;
    virtual bool IsStarConnection() const;
    virtual unsigned int IsSupportedProtocol(unsigned char protocolType) const;
    virtual bool IsBitRateCheckPermitted() const;
    virtual ConnectStationJob* CreateConnectStationJob();
    virtual DisconnectStationJob* CreateDisconnectStationJob();
    virtual void DeleteConnectStationJob(ConnectStationJob* job);
    virtual void DeleteDisconnectStationJob(DisconnectStationJob* job);
};

}
}
}

#endif
