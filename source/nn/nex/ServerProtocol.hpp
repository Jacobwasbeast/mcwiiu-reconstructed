#ifndef SOURCE_NN_NEX_SERVERPROTOCOL_HPP
#define SOURCE_NN_NEX_SERVERPROTOCOL_HPP

#include "nn/nex/Protocol.hpp"

namespace nn { namespace nex {

class ServerProtocol : public Protocol
{
public:
    explicit ServerProtocol(unsigned int protocol_type);
    ~ServerProtocol() override;
    _Type GetProtocolType() const override;
    bool UseAttemptCountMethod() const;
};

}}

#endif
