#include "nn/nex/TransportAdapter.hpp"

#include "nn/nex/StationURL.hpp"

namespace nn
{
namespace nex
{

TransportAdapter* g_transportAdapter = 0;

void TransportAdapter::ReleaseAddress(const StationURL&)
{
}

bool TransportAdapter::IsExportableURL(const StationURL&)
{
    return true;
}

unsigned int TransportAdapter::GetUniqueMachineID(const StationURL& rStationURL) const
{
    InetAddress address;
    const String urlAddress = rStationURL.GetAddress();

    address.SetAddress(urlAddress);

    return address.GetAddress();
}

void TransportAdapter::Load()
{
}

void TransportAdapter::Unload()
{
}

bool TransportAdapter::SetLocalAddress(const String& rAddress)
{
    m_localAddress.SetAddress(rAddress);
    return true;
}

bool TransportAdapter::SetPortNumber(unsigned short uiPortNumber)
{
    m_localAddress.SetPortNumber(uiPortNumber);
    return true;
}

void TransportAdapter::SetTransportMode(ETransportMode mode)
{
    m_transportMode = mode;
}

void TransportAdapter::Trace(unsigned long long)
{
}

bool TransportAdapter::IsSendMultiAddress(const InetAddress* pAddress)
{
    return pAddress->GetPortNumber() == 0;
}

}
}
