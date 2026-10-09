#include "nn/pia/lan/LanOutputStream.hpp"

#include <cstring>

#include <coreinit/memory.h>

#include "nn/pia/common/common.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/Socket.hpp"
#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn { namespace pia { namespace lan {

namespace
{
enum
{
    cConnectedStationState = 5
};
}

LanOutputStream::LanOutputStream()
    : common::SocketStreamBase(), common::IPacketOutput(), m_protocol(0)
{
}

LanOutputStream::~LanOutputStream()
{
}

Result LanOutputStream::OnStationConnectionEvent()
{
    if (!m_started)
        return 0;

    OSBlockSet(m_stationAddresses, 0, sizeof(m_stationAddresses));
    for (transport::Station** station = transport::StationManager::s_pInstance->m_stations.begin();
         station != transport::StationManager::s_pInstance->m_stations.end(); ++station)
    {
        if ((*station)->m_field20 != cConnectedStationState)
            continue;
        const int index = (*station)->m_field10;
        if (index >= transport::cStationNumMax)
            continue;
        common::SocketAddress address;
        address.SetInetAddress((*station)->m_stationAddress.m_inetAddress);
        m_stationAddresses[index] = address.GetSockAddrIn();
    }
    return 0;
}

Result LanOutputStream::CalculateCryptoParameter(unsigned char* output, unsigned int size,
                                                  common::Packet* packet)
{
    if (size < 12)
        return 0x10c07;
    const unsigned int protocolId = m_protocol == 0 ? 0 : m_protocol->GetProtocolIdentifier();
    common::serializeU32(output, protocolId);
    std::memcpy(output + 4, packet->m_packetData.m_header.m_cryptoNonce, 8);
    output[4] = packet->m_packetData.m_header.m_passId;
    return 0;
}

int LanOutputStream::CreateMulticastAddressArray(common::SockAddrIn* output, unsigned int mask)
{
    int count = 0;
    const transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation != 0)
    {
        const unsigned int localIndex = localStation->m_field10;
        for (unsigned int index = 0; index < transport::cStationNumMax; ++index)
        {
            if (index == localIndex || ((mask >> index) & 1) == 0)
                continue;
            const common::SockAddrIn& address = m_stationAddresses[index];
            if (address.address == 0 || address.port == 0)
                continue;
            output[count] = address;
            ++count;
        }
    }
    return count;
}

Result LanOutputStream::SendTo(const void* data, unsigned int size,
                               const common::InetAddress& address, unsigned char flags)
{
    const bool started = m_started;
    common::Socket* socket = m_socket;
    if (socket == 0)
        return started ? 0x10c08 : 0;
    int sent = 0;
    Result ttlResult = flags == 0 ? Result(0) : socket->SetTtl(flags);
    if (ttlResult != 0)
        return started ? ttlResult : Result(0);
    Result result = socket->SendTo(data, static_cast<int>(size), address, &sent);
    Result resetResult = flags == 0 ? Result(0) : socket->SetTtl(0);
    if (resetResult != 0)
        return started ? resetResult : Result(0);
    if (result != 0)
        return started ? result : Result(0);
    if (sent != static_cast<int>(size))
        return 0x10c08;
    if (m_protocol != 0)
        m_protocol->m_unknown618 = 1;
    return 0;
}

bool LanOutputStream::IsDestinationAddressIndividual(common::SockAddrIn* addresses, unsigned int count) const
{
    const unsigned int localAddress = m_protocol->m_address;
    for (unsigned int i = 0; i < count; ++i)
        if (localAddress == addresses[i].address)
            return false;
    for (unsigned int i = 0; i < count - 1; ++i)
        for (unsigned int j = i + 1; j <= count - 1; ++j)
            if (addresses[i].address == addresses[j].address)
                return false;
    return true;
}

Result LanOutputStream::SendToMulti(const void* data, unsigned int size,
                                    const common::SockAddrIn* addresses, int count)
{
    const bool started = m_started;
    common::Socket* socket = m_socket;
    if (socket == 0)
        return started ? 0x10c08 : 0;
    int sent = 0;
    Result result = socket->SendToMulti(data, static_cast<int>(size), addresses, count, &sent);
    if (result == 0)
        return 0x10c08;
    return started ? result : Result(0);
}

namespace
{
Result TraceSendResult(const common::Packet& packet, Result result)
{
    if (result != 0)
        packet.Trace(result == 0x10c07 ? 0 : 0x02000000, result == 0x10c07);
    return result;
}
}

Result LanOutputStream::Write(const common::Packet& packet)
{
    if (!m_started)
        return 0;
    common::InetAddress address;
    if (packet.m_stationIndex != 0xff)
        address = packet.m_field5f4.m_inetAddress;
    if (address.IsValid())
    {
        return TraceSendResult(packet, SendTo(packet.m_packetData.m_data, packet.m_size, address, packet.m_field600));
    }
    common::SockAddrIn addresses[32];
    const int count = CreateMulticastAddressArray(addresses, packet.m_stationBitmap);
    if (count == 0)
        return 0;
    if (count == 1)
    {
        common::SocketAddress socketAddress;
        socketAddress.SetSockAddrIn(addresses[0]);
        common::InetAddress address;
        socketAddress.GetInetAddress(&address);
        return TraceSendResult(packet, SendTo(packet.m_packetData.m_data, packet.m_size, address, packet.m_field600));
    }
    if (IsDestinationAddressIndividual(addresses, count) == false || m_protocol == 0 ||
        !m_protocol->IsRelayEnabled())
        return TraceSendResult(packet, SendToMulti(packet.m_packetData.m_data, packet.m_size, addresses, count));
    common::InetAddress relay(m_protocol->m_broadcastAddress, m_protocol->m_port);
    relay.m_port = 40000;
    return TraceSendResult(packet, SendTo(packet.m_packetData.m_data, packet.m_size, relay, packet.m_field600));
}

void LanOutputStream::RegisterLanProtocol(LanProtocol* protocol)
{
    m_protocol = protocol;
}

void LanOutputStream::UnregisterLanProtocol()
{
    m_protocol = 0;
}

unsigned int LanOutputStream::GetMulticastDestinationNum() const
{
    return 16;
}

unsigned int LanOutputStream::IsBroadcastSupported() const
{
    return true;
}

}}}
