#include "nn/nex/PacketOut.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Timeout.hpp"

namespace nn
{
namespace nex
{

PacketOut::PacketOut(PRUDPEndPoint* pEndPoint, PacketType type, unsigned short uiFlags,
    Buffer* pPayload, unsigned int uiSize)
    : Packet()
    , m_unknown120(0)
    , m_unknown124(0)
    , m_unknown138(0)
{
    Initialize(pEndPoint, type, uiFlags, pPayload, uiSize,
        (uiFlags & PACKET_FLAG_NEED_ACK) ? new Timeout() : 0);
}

PacketOut::~PacketOut()
{
    if (m_pTimeout != 0)
    {
        delete m_pTimeout;
        m_pTimeout = 0;
    }
}

void PacketOut::Initialize(PRUDPEndPoint* pEndPoint, PacketType type, unsigned short uiFlags,
    Buffer* pPayload, unsigned int uiSize, Timeout* pTimeout)
{
    m_unknown11a = 0;
    m_unknown120 = 0;
    m_uiTypeAndFlags = m_uiTypeAndFlags | (type & PACKET_TYPE_MASK) |
        (uiFlags & static_cast<unsigned short>(~PACKET_TYPE_MASK));
    m_unknown124 = 0;
    m_pEndPoint = pEndPoint;
    m_unknown140 = uiSize;
    m_unknown118 = 0;
    m_unknown138 = 0;
    if (pPayload != 0)
    {
        pPayload->AddRef();
        SetPayload(pPayload);
    }
    m_unknown130 = 0;
    m_pTimeout = pTimeout;
    m_unknown12c = 0;
    m_unknown129 = false;
    m_unknown128 = false;
}

bool PacketOut::IsValid()
{
    if ((m_uiTypeAndFlags & PACKET_FLAG_NEED_ACK) && m_pTimeout == 0)
        return false;

    return true;
}

}
}
