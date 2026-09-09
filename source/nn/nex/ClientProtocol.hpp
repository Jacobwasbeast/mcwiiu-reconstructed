#ifndef SOURCE_NN_NEX_CLIENTPROTOCOL_HPP
#define SOURCE_NN_NEX_CLIENTPROTOCOL_HPP

#include "nn/nex/Protocol.hpp"

namespace nn { namespace nex {

class ClientProtocol : public Protocol
{
public:
    explicit ClientProtocol(unsigned int protocol_type);
    virtual ~ClientProtocol();
    _Type GetProtocolType() const override;

private:
    void* m_client_state;
};

}}

#endif
