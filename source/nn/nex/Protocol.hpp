#ifndef SOURCE_NN_NEX_PROTOCOL_HPP
#define SOURCE_NN_NEX_PROTOCOL_HPP

#include <cstdint>
#include "nn/nex/SystemComponent.hpp"

namespace nn { namespace nex {

class Protocol : public SystemComponent
{
public:
    enum _Type
    {
        TYPE_CLIENT = 0,
        TYPE_SERVER = 1
    };

    explicit Protocol(unsigned int protocol_type);
    virtual ~Protocol();
    virtual _Type GetProtocolType() const;
    void SetProtocolID(unsigned short protocol_id);
    unsigned short GetProtocolID() const;

protected:
    unsigned short m_protocol_id;
    void* m_end_point;
    void* m_request_broker;
    unsigned int m_protocol_type;
    void* m_outgoing_connection;
    unsigned char m_local_loopback;
    unsigned int m_caller_pid;
    unsigned int m_caller_cid;
};

}}

#endif
