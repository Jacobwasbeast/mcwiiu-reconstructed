#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_PROTOCOLMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_PROTOCOLMANAGER_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/ListBase.hpp"
#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/Reconstruction.hpp"

namespace nn { namespace pia { namespace transport {

class ProtocolManager
{
public:
    ProtocolManager();
    virtual ~ProtocolManager();
    void Finalize();
    void Initialize(NetworkFactory* networkFactory);
    Result Startup(PacketHandler* packetHandler);
    Result StartupProtocols();
    Result Dispatch();
    void Cleanup();
    void CleanupProtocols();
    void CleanupProtocolsWithStationAddress();
    Result StartupProtocolsWithStationAddress(const common::StationAddress& address);
    UnreliableProtocol* AllocProtocol(unsigned int size);
    ProtocolId CreateProtocolImpl(Protocol* protocol, ProtocolId id);
    Protocol* SearchProtocol(ProtocolId id, unsigned char type);
    void DestroyProtocol(unsigned short id);
private:
    NetworkFactory* m_networkFactory;
    common::ListBase m_protocols;
    int m_nodeOffset;
public:
    unsigned char m_started;
};

extern ProtocolManager* g_piaProtocolManager;

}}}

#endif
