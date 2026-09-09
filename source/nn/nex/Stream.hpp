#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STREAM_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Packet;

class Stream : public RootObject
{
public:
    enum Type
    {
        RendezVous = 2,
        NAT = 5,
        Routing = 8,
        RendezVousSecure = 10,
        Relay = 11
    };

    explicit Stream(Type type);
    virtual ~Stream();

    virtual void ReceiveIncomingPacket(unsigned short uiPort, unsigned char uiStreamID,
        Packet* pPacket) = 0;
    virtual void DoWork() = 0;
    virtual bool IsDuplicateReorderingPacket(Packet* pPacket);

protected:
    Type m_Type;
};

}
}

#endif
