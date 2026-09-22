#include "nn/pia/lan/LanInputStream.hpp"
#include <cstring>

namespace nn { namespace pia { namespace lan {

LanInputStream::LanInputStream() : common::SocketStreamBase(), inputSocket(0), broadcastSocket(0), protocol(0), bytesRead(0), hasIgnoreAddress(false) {}
LanInputStream::~LanInputStream() {}
void LanInputStream::readFromSocket(common::Socket* socket, common::Packet* packet)
{
    inputSocket = socket;
    if (inputSocket != 0 && packet != 0)
    {
        unsigned char* payload = packet->AssignPayload(common::Packet::cPayloadSizeMax);
        if (payload != 0)
            bytesRead = static_cast<unsigned int>(inputSocket->RecvFrom(payload, common::Packet::cPayloadSizeMax, 0, 0, 0));
    }
}
int LanInputStream::Read(common::Packet* packet)
{
    if (inputSocket == 0 || packet == 0)
        return 0x10c07;
    readFromSocket(inputSocket, packet);
    return bytesRead == 0 ? 0x10c08 : 0;
}
void LanInputStream::CalculateCryptoParameter(unsigned char* output, unsigned int size, common::Packet* packet)
{
    if (output != 0 && packet != 0)
        std::memcpy(output, packet->m_packetData.m_header.m_field18, size < sizeof(packet->m_packetData.m_header.m_field18) ? size : sizeof(packet->m_packetData.m_header.m_field18));
}
void LanInputStream::RegisterLanProtocol(LanProtocol* value) { protocol = value; }
void LanInputStream::UnregisterLanProtocol() { protocol = 0; }
void LanInputStream::ImportBroadcastSocket(common::Socket* socket) { broadcastSocket = socket; }
void LanInputStream::AddIgnoreAddress(const common::InetAddress&) { hasIgnoreAddress = true; }
void LanInputStream::ResetIgnoreAddress() { hasIgnoreAddress = false; }
void LanInputStream::RemoveIgnoreAddess(const common::InetAddress&) { hasIgnoreAddress = false; }

}}}
