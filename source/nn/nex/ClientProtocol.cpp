#include "nn/nex/ClientProtocol.hpp"

namespace nn { namespace nex {

ClientProtocol::ClientProtocol(unsigned int protocol_type)
    : Protocol(protocol_type), m_client_state(NULL)
{
}

ClientProtocol::~ClientProtocol()
{
}

Protocol::_Type ClientProtocol::GetProtocolType() const
{
    return TYPE_CLIENT;
}

}}
