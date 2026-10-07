#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/ProtocolManager.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"
#include <new>
#include <string.h>

namespace nn { namespace pia { namespace transport {


unsigned char g_piaTransportInitialized = 0;
unsigned char g_piaTransportInSetupMode = 0;
ProtocolId g_piaProtocolDefaultType;
ProtocolManager* g_piaProtocolManager = 0;

unsigned char IsInitialized()
{
    return g_piaTransportInitialized;
}

unsigned char IsInSetupMode()
{
    return g_piaTransportInSetupMode;
}


Protocol::Protocol()
    : mProtocolId(g_piaProtocolDefaultType)
{
    prev = 0;
    next = 0;
}

Protocol::~Protocol()
{
}

void Protocol::Trace(unsigned long long) const
{
}

unsigned short Protocol::GetProtocolType() const
{
    return 0;
}

Result Protocol::StartupWithStationAddress(const common::StationAddress&)
{
    return 0;
}

Result Protocol::Startup()
{
    return 0;
}

void Protocol::SetPort(unsigned char port)
{
    mProtocolId = ProtocolId(static_cast<unsigned char>(GetProtocolType()), port);
}

unsigned int Protocol::GetStationPacketHandler()
{
    return mStationPacketHandler;
}

void Protocol::CleanupWithStationAddress()
{
}

void Protocol::Cleanup()
{
}

Result Protocol::Dispatch()
{
    return 0;
}

Result Protocol::UpdateProtocolEvent(const ProtocolEvent&)
{
    return 0;
}

unsigned int Protocol::IsProtocolFilteringEnabled() const
{
    return 1;
}

}}}
