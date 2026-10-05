#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFERPACKETOUT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFERPACKETOUT_HPP

#include "nn/nex/PacketOut.hpp"

namespace nn
{
namespace nex
{

class PacketBufferPacketOut : public PacketOut
{
public:
    PacketBufferPacketOut(PRUDPEndPoint* pEndPoint, PacketType type, unsigned short uiFlags,
        Buffer* pPayload, unsigned int uiSize)
        : PacketOut(pEndPoint, type, uiFlags, pPayload, uiSize)
    {
    }

    virtual ~PacketBufferPacketOut();
};

}
}

#endif
