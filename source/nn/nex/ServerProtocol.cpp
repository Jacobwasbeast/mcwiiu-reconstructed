#include "nn/nex/ServerProtocol.hpp"

namespace nn { namespace nex {

ServerProtocol::ServerProtocol(unsigned int protocol_type)
    : Protocol(protocol_type)
{
}

ServerProtocol::~ServerProtocol()
{
}

Protocol::_Type ServerProtocol::GetProtocolType() const
{
    return TYPE_SERVER;
}

bool ServerProtocol::UseAttemptCountMethod() const
{
    return false;
}

}}
