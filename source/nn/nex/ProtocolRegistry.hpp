#ifndef SOURCE_NN_NEX_PROTOCOLREGISTRY_HPP
#define SOURCE_NN_NEX_PROTOCOLREGISTRY_HPP

#include <cstddef>
#include <map>
#include <utility>
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/Protocol.hpp"

namespace nn { namespace nex {

class ProtocolRegistry : public RootObject
{
public:
    ProtocolRegistry();
    virtual ~ProtocolRegistry();
    unsigned int RegisterProtocol(Protocol* protocol);
    void RegisterProtocol(unsigned short protocolId, Protocol* protocol);
    Protocol* GetProtocol(unsigned short protocolId, Protocol::_Type type);
    Protocol* GetServerProtocol(unsigned short protocolId);
    unsigned long long UnregisterProtocol(Protocol* protocol);

private:
    typedef std::pair<const unsigned short, Protocol*> ProtocolEntry;
    typedef std::map<unsigned short, Protocol*, std::less<unsigned short>, MemAllocator<ProtocolEntry> > ProtocolMap;
    ProtocolMap m_protocols;
    ProtocolMap m_serverProtocols;
};

}}

#endif