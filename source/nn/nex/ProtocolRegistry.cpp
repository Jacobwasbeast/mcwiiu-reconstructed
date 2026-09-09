#include "nn/nex/ProtocolRegistry.hpp"

namespace nn { namespace nex {

ProtocolRegistry::ProtocolRegistry()
    : m_protocols(), m_serverProtocols()
{
}

ProtocolRegistry::~ProtocolRegistry()
{
    m_protocols.clear();
    m_serverProtocols.clear();
}

unsigned int ProtocolRegistry::RegisterProtocol(Protocol* protocol)
{
    if (protocol == 0)
        return 0;
    const unsigned short id = protocol->GetProtocolID();
    ProtocolMap& protocols = protocol->GetProtocolType() == Protocol::TYPE_SERVER
        ? m_serverProtocols : m_protocols;
    protocols[id] = protocol;
    return 1;
}

void ProtocolRegistry::RegisterProtocol(unsigned short protocolId, Protocol* protocol)
{
    if (protocol == 0)
        return;
    protocol->SetProtocolID(protocolId);
    RegisterProtocol(protocol);
}

Protocol* ProtocolRegistry::GetProtocol(unsigned short protocolId, Protocol::_Type type)
{
    ProtocolMap& protocols = type == Protocol::TYPE_SERVER ? m_serverProtocols : m_protocols;
    ProtocolMap::iterator it = protocols.find(protocolId);
    return it == protocols.end() ? 0 : it->second;
}

Protocol* ProtocolRegistry::GetServerProtocol(unsigned short protocolId)
{
    ProtocolMap::iterator it = m_serverProtocols.find(protocolId);
    return it == m_serverProtocols.end() ? 0 : it->second;
}

unsigned long long ProtocolRegistry::UnregisterProtocol(Protocol* protocol)
{
    if (protocol == 0)
        return 0;
    const unsigned short id = protocol->GetProtocolID();
    ProtocolMap& protocols = protocol->GetProtocolType() == Protocol::TYPE_SERVER
        ? m_serverProtocols : m_protocols;
    ProtocolMap::iterator it = protocols.find(id);
    if (it != protocols.end() && it->second == protocol)
        protocols.erase(it);
    return 0;
}

}}