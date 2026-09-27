#include "nn/pia/lan/LanOutputStream.hpp"

#include <cstring>

#include "nn/pia/common/common.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/Socket.hpp"
#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn { namespace pia { namespace lan {

LanOutputStream::LanOutputStream() : common::SocketStreamBase(), m_protocol(0)
{
}

LanOutputStream::~LanOutputStream()
{
}

void LanOutputStream::OnStationConnectionEvent()
{
    std::memset(m_stationAddresses, 0, sizeof(m_stationAddresses));
    for (unsigned int i = 0; i < transport::g_piaStationManager->m_stations.size; ++i)
    {
        transport::Station* station = transport::g_piaStationManager->m_stations.data[i];
        if (station->m_field20 != 5 || station->m_field10 >= 32)
            continue;
        common::SocketAddress address;
        address.Init();
        address.SetInetAddress(station->m_stationAddress);
        address.GetSockAddrIn();
        std::memcpy(m_stationAddresses + station->m_field10 * sizeof(common::SockAddrIn),
                    address.GetSockAddrInPointer(), sizeof(common::SockAddrIn));
    }
}

Result LanOutputStream::CalculateCryptoParameter(unsigned char* output, unsigned int size,
                                                  common::Packet* packet)
{
    if (size < 12)
        return 0x10c07;
    const unsigned int protocolId = m_protocol == 0 ? 0 : m_protocol->GetProtocolIdentifier();
    common::serializeU32(output, protocolId);
    std::memcpy(output + 4, packet->m_packetData.m_header.m_field10, 8);
    output[4] = packet->m_packetData.m_header.m_field09;
    return 0;
}

int LanOutputStream::CreateMulticastAddressArray(common::SockAddrIn* output, unsigned int mask)
{
    const unsigned int excluded = transport::g_piaStationManager->m_localStation->m_field10;
    int count = 0;
    for (unsigned int index = 0; index != 32; ++index)
    {
        if (index == excluded || (mask & (1u << index)) == 0)
            continue;
        const unsigned char* entry = m_stationAddresses + index * sizeof(common::SockAddrIn);
        if (*reinterpret_cast<const unsigned int*>(entry + 4) == 0 ||
            *reinterpret_cast<const unsigned short*>(entry + 2) == 0)
            continue;
        std::memcpy(output + count, entry, sizeof(common::SockAddrIn));
        ++count;
    }
    return count;
}

Result LanOutputStream::SendTo(const void* data, unsigned int size,
                               const common::InetAddress& address, unsigned char flags)
{
    const bool started = *reinterpret_cast<const unsigned char*>(this) != 0;
    common::Socket* socket = *reinterpret_cast<common::Socket**>(
        reinterpret_cast<unsigned char*>(this) + 4);
    if (socket == 0)
        return started ? 0x10c08 : 0;
    int sent = 0;
    Result ttlResult = flags == 0 ? 0 : socket->SetTtl(flags);
    if (ttlResult != 0)
        return started ? ttlResult : 0;
    Result result = socket->SendTo(data, static_cast<int>(size), address, &sent);
    Result resetResult = flags == 0 ? 0 : socket->SetTtl(0);
    if (resetResult != 0)
        return started ? resetResult : 0;
    if (result != 0)
        return started ? result : 0;
    if (sent != static_cast<int>(size))
        return 0x10c08;
    if (m_protocol != 0)
        *reinterpret_cast<unsigned char*>(reinterpret_cast<unsigned char*>(m_protocol) + 0x618) = 1;
    return 0;
}

bool LanOutputStream::IsDestinationAddressIndividual(common::SockAddrIn* addresses, unsigned int count) const
{
    if (addresses == 0 || m_protocol == 0)
        return count <= 1;
    const unsigned int localAddress = *reinterpret_cast<const unsigned int*>(
        reinterpret_cast<const unsigned char*>(m_protocol) + 0x18);
    for (unsigned int i = 0; i != count; ++i)
        if (*reinterpret_cast<const unsigned int*>(addresses[i].bytes + 4) == localAddress)
            return false;
    for (unsigned int i = 0; i != count; ++i)
        for (unsigned int j = i + 1; j != count; ++j)
            if (*reinterpret_cast<const unsigned int*>(addresses[i].bytes + 4) ==
                *reinterpret_cast<const unsigned int*>(addresses[j].bytes + 4))
                return false;
    return true;
}

Result LanOutputStream::SendToMulti(const void* data, unsigned int size,
                                    const common::SockAddrIn* addresses, int count)
{
    const bool started = *reinterpret_cast<const unsigned char*>(this) != 0;
    common::Socket* socket = *reinterpret_cast<common::Socket* const*>(
        reinterpret_cast<const unsigned char*>(this) + 4);
    if (socket == 0)
        return started ? 0x10c08 : 0;
    int sent = 0;
    Result result = socket->SendToMulti(data, static_cast<int>(size), addresses, count, &sent);
    if (result == 0)
        return 0x10c08;
    return started ? result : 0;
}

Result LanOutputStream::Write(const common::Packet& packet)
{
    if (*reinterpret_cast<const unsigned char*>(this) == 0)
        return 0;
    common::InetAddress address;
    const auto traceFailure = [&packet](Result result)
    {
        if (result != 0)
            packet.Trace(result == 0x10c07 ? 0 : 0x02000000, result == 0x10c07);
        return result;
    };
    if (packet.m_stationIndex != 0xff)
        address = packet.m_field5f4;
    if (address.IsValid())
    {
        return traceFailure(SendTo(packet.m_packetData.m_data, packet.m_size, address, packet.m_field600));
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
        return traceFailure(SendTo(packet.m_packetData.m_data, packet.m_size, address, packet.m_field600));
    }
    if (IsDestinationAddressIndividual(addresses, count) == false || m_protocol == 0 ||
        !m_protocol->IsRelayEnabled())
        return traceFailure(SendToMulti(packet.m_packetData.m_data, packet.m_size, addresses, count));
    common::InetAddress relay(*reinterpret_cast<const common::InetAddress*>(
        reinterpret_cast<const unsigned char*>(m_protocol) + 0x1c));
    relay.m_port = 40000;
    return traceFailure(SendTo(packet.m_packetData.m_data, packet.m_size, relay, packet.m_field600));
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

bool LanOutputStream::IsBroadcastSupported() const
{
    return true;
}

}}}
