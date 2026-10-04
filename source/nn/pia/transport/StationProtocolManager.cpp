#include "nn/pia/transport/StationProtocolManager.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/transport/RttProtocol.hpp"
#include "nn/pia/transport/StationProtocol.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>

namespace nn { namespace pia { namespace transport {

StationProtocolManager* g_piaStationProtocolManager = 0;

namespace {

enum { cStationProtocolType = 20, cRttProtocolType = 88 };

unsigned char* g_stationProtocolManagerStorage = 0;

}

StationProtocolManager::StationProtocolManager()
    : m_stationProtocolId(0), m_rttProtocolId(0)
{
}

StationProtocolManager::~StationProtocolManager()
{
}

Result StationProtocolManager::CreateInstance()
{
    if (IsInitialized() == 0)
        return cResultNotInitialized;
    if (IsInSetupMode() == 0)
        return cResultInvalidState;
    if (g_piaStationProtocolManager != 0)
        return cResultAlreadyExists;
    g_stationProtocolManagerStorage = static_cast<unsigned char*>(pead::allocateFromSeadHeap(
        sizeof(StationProtocolManager), common::HeapManager::GetHeap(), 8));
    if (g_stationProtocolManagerStorage == 0)
        return cResultOutOfMemory;
    for (unsigned int index = 0; index < sizeof(StationProtocolManager); ++index)
        g_stationProtocolManagerStorage[index] = 0;
    g_piaStationProtocolManager = new (g_stationProtocolManagerStorage) StationProtocolManager();
    return 0;
}

void StationProtocolManager::DestroyInstance()
{
    if (g_piaStationProtocolManager != 0)
    {
        g_piaStationProtocolManager->~StationProtocolManager();
        g_piaStationProtocolManager = 0;
    }
    if (g_stationProtocolManagerStorage != 0)
    {
        pead::getArraySize(g_stationProtocolManagerStorage);
        pead::freeToSeadHeap(g_stationProtocolManagerStorage);
        g_stationProtocolManagerStorage = 0;
    }
}

Result StationProtocolManager::Initialize(unsigned short stationNum, int initialPulseInterval)
{
    Transport* transport = g_piaTransport;
    if (transport == 0)
        return cResultInvalidState;
    ProtocolManager& manager = transport->m_protocolManager;
    if (!manager.m_started)
    {
        UnreliableProtocol* storage = manager.AllocProtocol(sizeof(StationProtocol));
        StationProtocol* stationProtocol = storage != 0
            ? new (static_cast<void*>(storage)) StationProtocol() : 0;
        const ProtocolId stationId = manager.CreateProtocolImpl(
            stationProtocol, ProtocolId(cStationProtocolType << 8));
        m_stationProtocolId = stationId;
        if (stationId != 0)
        {
            static_cast<StationProtocol*>(manager.SearchProtocol(stationId, cStationProtocolType))
                ->Initialize();
            if (!manager.m_started)
            {
                UnreliableProtocol* rttStorage = manager.AllocProtocol(sizeof(RttProtocol));
                RttProtocol* rttProtocol = rttStorage != 0
                    ? new (static_cast<void*>(rttStorage)) RttProtocol() : 0;
                const ProtocolId rttId = manager.CreateProtocolImpl(
                    rttProtocol, ProtocolId(cRttProtocolType << 8));
                m_rttProtocolId = rttId;
                if (rttId != 0)
                {
                    static_cast<RttProtocol*>(manager.SearchProtocol(rttId, cRttProtocolType))
                        ->Initialize(stationNum, initialPulseInterval);
                    return 0;
                }
            }
            else
                m_rttProtocolId = 0;
            return cResultInvalidState;
        }
    }
    else
        m_stationProtocolId = 0;
    return cResultInvalidState;
}

void StationProtocolManager::Finalize()
{
    Transport* transport = g_piaTransport;
    if (transport == 0)
        return;
    ProtocolManager& manager = transport->m_protocolManager;
    if (m_rttProtocolId != 0)
    {
        static_cast<RttProtocol*>(manager.SearchProtocol(m_rttProtocolId, cRttProtocolType))
            ->Finalize();
        manager.DestroyProtocol(m_rttProtocolId);
        m_rttProtocolId = 0;
    }
    if (m_stationProtocolId != 0)
    {
        static_cast<StationProtocol*>(manager.SearchProtocol(m_stationProtocolId, cStationProtocolType))
            ->Finalize();
        manager.DestroyProtocol(m_stationProtocolId);
        m_stationProtocolId = 0;
    }
}

StationProtocol* StationProtocolManager::GetStationProtocol()
{
    return static_cast<StationProtocol*>(
        g_piaTransport->m_protocolManager.SearchProtocol(m_stationProtocolId, cStationProtocolType));
}

RttProtocol* StationProtocolManager::GetRttProtocol()
{
    return static_cast<RttProtocol*>(
        g_piaTransport->m_protocolManager.SearchProtocol(m_rttProtocolId, cRttProtocolType));
}

void StationProtocolManager::Trace(unsigned long long) const
{
}

}}}
