#include "nn/pia/lan/LanInputStream.hpp"

#include <cstring>

#include "nn/pia/String.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/common.hpp"
#include "nn/pia/lan/LanProtocol.hpp"

namespace nn { namespace pia { namespace lan {

namespace
{
void traceIgnoredAddress(const common::InetAddress& address)
{
    common::FixedString<32> text;
    address.GetAddressString(&text);
}
}

LanInputStream::LanInputStream()
    : common::SocketStreamBase(), common::IPacketInput(), m_ignoreAddresses(),
      m_broadcastSocket(0), m_broadcastReceived(false), m_protocol(0)
{
}

LanInputStream::~LanInputStream()
{
    m_broadcastSocket = 0;
}

Result LanInputStream::readFromSocket(common::Socket* socket, common::Packet* packet)
{
    common::InetAddress source;
    unsigned char ancillary = 0;
    Result result = socket->RecvFrom(packet->m_packetData.m_data, common::Packet::cDataSizeMax,
                                     &source, &ancillary,
                                     reinterpret_cast<int*>(&packet->m_size));
    if (result != 0)
    {
        if (!m_started)
            return 0x2c09;
        return result;
    }
    if (!source.IsValid())
    {
        traceIgnoredAddress(source);
        return 0x2c09;
    }
    for (unsigned int index = 0; index < cIgnoreAddressMax; ++index)
    {
        if (m_ignoreAddresses[index].GetKey() == source.GetKey())
        {
            traceIgnoredAddress(source);
            return 0x2c09;
        }
    }
    common::StationAddress stationAddress;
    stationAddress.SetInetAddress(source);
    packet->m_field608 = stationAddress;
    packet->m_field600 = ancillary;
    return 0;
}

Result LanInputStream::Read(common::Packet* packet)
{
    if (!m_started)
        return 0x2c09;

    Result result;
    if (!m_protocol->m_broadcastSocketBound)
    {
        result = readFromSocket(m_socket, packet);
    }
    else if (!m_broadcastReceived)
    {
        result = readFromSocket(m_socket, packet);
        if (result != 0x2c09)
        {
            if (result == 0)
            {
                m_broadcastReceived = true;
                m_protocol->m_unknown618 = 1;
            }
            return result;
        }
        result = readFromSocket(m_broadcastSocket, packet);
    }
    else
    {
        result = readFromSocket(m_broadcastSocket, packet);
        if (result == 0x2c09)
        {
            result = readFromSocket(m_socket, packet);
        }
        else
        {
            if (result != 0)
                return result;
            m_broadcastReceived = false;
        }
    }
    if (result != 0)
        return result;
    m_protocol->m_unknown618 = 1;
    return result;
}

Result LanInputStream::CalculateCryptoParameter(unsigned char* parameter, unsigned int size,
                                                common::Packet* packet)
{
    if (size < 12)
        return 0x10c07;
    common::serializeU32(parameter, packet->m_field608.m_inetAddress.m_address);
    std::memcpy(parameter + 4, packet->m_packetData.m_header.m_cryptoNonce, 8);
    parameter[4] = packet->m_packetData.m_header.m_passId;
    return 0;
}

void LanInputStream::RegisterLanProtocol(LanProtocol* protocol)
{
    m_protocol = protocol;
}

void LanInputStream::UnregisterLanProtocol()
{
    m_protocol = 0;
}

Result LanInputStream::ImportBroadcastSocket(common::Socket* socket)
{
    m_broadcastSocket = socket;
    return 0;
}

void LanInputStream::AddIgnoreAddress(const common::InetAddress& address)
{
    if (!address.IsValid())
        return;
    for (unsigned int index = 0; index < cIgnoreAddressMax; ++index)
    {
        if (m_ignoreAddresses[index].GetKey() == address.GetKey())
            return;
    }
    for (unsigned int index = 0; index < cIgnoreAddressMax; ++index)
    {
        if (!m_ignoreAddresses[index].IsValid())
        {
            m_ignoreAddresses[index] = address;
            return;
        }
    }
}

void LanInputStream::ResetIgnoreAddress()
{
    for (unsigned int index = 0; index < cIgnoreAddressMax; ++index)
    {
        m_ignoreAddresses[index].m_address = 0;
        m_ignoreAddresses[index].m_port = 0;
    }
}

void LanInputStream::RemoveIgnoreAddess(const common::InetAddress& address)
{
    for (unsigned int index = 0; index < cIgnoreAddressMax; ++index)
    {
        if (m_ignoreAddresses[index].GetKey() == address.GetKey())
        {
            m_ignoreAddresses[index].m_address = 0;
            m_ignoreAddresses[index].m_port = 0;
        }
    }
}

}}}
