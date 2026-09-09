#ifndef SOURCE_NN_NEX_PROTOCOLREGISTRY_HPP
#define SOURCE_NN_NEX_PROTOCOLREGISTRY_HPP

#include <cstddef>
#include <map>
#include <utility>
#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/Protocol.hpp"

namespace nn { namespace nex {

template <typename T>
class ProtocolRegistryAllocator
{
public:
    typedef T value_type;
    ProtocolRegistryAllocator() {}
    template <typename U> ProtocolRegistryAllocator(const ProtocolRegistryAllocator<U>&) {}
    T* allocate(std::size_t count)
    {
        return static_cast<T*>(MemoryManager::Allocate(static_cast<unsigned int>(count * sizeof(T))));
    }
    T* allocate(std::size_t count, const void*)
    {
        return allocate(count);
    }
    void deallocate(T* pointer, std::size_t)
    {
        MemoryManager::Free(pointer);
    }
    template <typename U> struct rebind { typedef ProtocolRegistryAllocator<U> other; };
};

template <typename T, typename U>
inline bool operator==(const ProtocolRegistryAllocator<T>&, const ProtocolRegistryAllocator<U>&) { return true; }
template <typename T, typename U>
inline bool operator!=(const ProtocolRegistryAllocator<T>&, const ProtocolRegistryAllocator<U>&) { return false; }

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
    typedef std::map<unsigned short, Protocol*, std::less<unsigned short>, ProtocolRegistryAllocator<ProtocolEntry> > ProtocolMap;
    ProtocolMap m_protocols;
    ProtocolMap m_serverProtocols;
};

}}

#endif