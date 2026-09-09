#include "nn/nex/Protocol.hpp"

namespace nn { namespace nex {

Protocol::Protocol(unsigned int protocol_type)
    : SystemComponent(*reinterpret_cast<const String*>(0)),
      m_protocol_id(0), m_end_point(NULL), m_request_broker(NULL),
      m_protocol_type(protocol_type), m_outgoing_connection(NULL), m_local_loopback(0),
      m_caller_pid(0), m_caller_cid(0)
{
}

Protocol::~Protocol()
{
}

Protocol::_Type Protocol::GetProtocolType() const
{
    return static_cast<Protocol::_Type>(m_protocol_type);
}

void Protocol::SetProtocolID(unsigned short protocol_id)
{
    m_protocol_id = protocol_id;
}

unsigned short Protocol::GetProtocolID() const
{
    return m_protocol_id;
}

}}
