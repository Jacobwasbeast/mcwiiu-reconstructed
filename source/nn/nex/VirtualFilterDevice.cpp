#include "nn/nex/VirtualFilterDevice.hpp"

namespace nn
{
namespace nex
{

bool VirtualFilterDevice::Match(const InetAddress& rAddress)
{
    if (m_addresses.find(rAddress) == m_addresses.end())
    {
        InetAddress anyAddress(0u, rAddress.GetPortNumber());
        if (m_addresses.find(anyAddress) == m_addresses.end())
            return false;
    }
    return true;
}

}
}
