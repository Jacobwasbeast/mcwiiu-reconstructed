#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_INPUT_STREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_INPUT_STREAM_HPP

#include "nn/pia/common/SocketStreamBase.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/Socket.hpp"
#include "nn/pia/common/InetAddress.hpp"

namespace nn { namespace pia { namespace lan { class LanProtocol;

class LanInputStream : public common::SocketStreamBase
{
public:
    LanInputStream();
    virtual ~LanInputStream();
    void readFromSocket(common::Socket* socket, common::Packet* packet);
    int Read(common::Packet* packet);
    void CalculateCryptoParameter(unsigned char* output, unsigned int size, common::Packet* packet);
    void RegisterLanProtocol(LanProtocol* protocol);
    void UnregisterLanProtocol();
    void ImportBroadcastSocket(common::Socket* socket);
    void AddIgnoreAddress(const common::InetAddress& address);
    void ResetIgnoreAddress();
    void RemoveIgnoreAddess(const common::InetAddress& address);

private:
    common::Socket* inputSocket;
    common::Socket* broadcastSocket;
    LanProtocol* protocol;
    unsigned int bytesRead;
    bool hasIgnoreAddress;
};

}}}

#endif
