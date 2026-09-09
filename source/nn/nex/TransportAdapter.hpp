#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTADAPTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTADAPTER_HPP

#include "nn/nex/InetAddress.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class InterfaceInfo;
class StationURL;

template <typename T>
class qProtectedList;

class TransportAdapter : public RootObject
{
public:
    enum ETransportMode
    {
    };

    TransportAdapter();
    virtual ~TransportAdapter();

    virtual bool GetLocalURLs(qList<StationURL>* pURLs);
    virtual void ResolveAddress(CallContext* pCallContext, StationURL* pStationURL);
    virtual void ReleaseAddress(const StationURL& rStationURL);
    virtual void SortAndFilterTargetURLs(const qProtectedList<StationURL>& rTargetURLs,
        qList<StationURL>* pURLs);
    virtual bool IsExportableURL(const StationURL& rStationURL);
    virtual unsigned int GetUniqueMachineID(const StationURL& rStationURL) const;
    virtual void Load();
    virtual void Unload();
    virtual bool SetNetworkInterfaceInfo(InterfaceInfo* pInterfaceInfo);
    virtual bool SetLocalAddress(const String& rAddress);
    virtual bool SetPortNumber(unsigned short uiPortNumber);

    void SetTransportMode(ETransportMode mode);
    void Trace(unsigned long long uiFilter);

    static bool IsSendMultiAddress(const InetAddress* pAddress);
    static void TraceInetAddress(unsigned long long uiFilter, InetAddress* pAddress);

protected:
    ETransportMode m_transportMode;
    InetAddress m_broadcastAddress;
    InetAddress m_multicastAddress;
    InetAddress m_localAddress;
    unsigned int m_uiFlags;
};

extern TransportAdapter* g_transportAdapter;

}
}

#endif
