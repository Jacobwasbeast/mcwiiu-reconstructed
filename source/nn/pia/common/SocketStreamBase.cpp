#include "nn/pia/common/SocketStreamBase.hpp"
#include <string.h>

namespace nn { namespace pia { namespace common {

SocketStreamBase::SocketStreamBase() : m_started(false), m_socket(0)
{
}

SocketStreamBase::~SocketStreamBase()
{
    m_socket = 0;
}

Result SocketStreamBase::Startup()
{
    m_started = true;
    void* startupState = ::operator new(4);
    if (startupState != 0)
        memset(startupState, 0, 4);
    return 0;
}

void SocketStreamBase::Cleanup()
{
    m_started = false;
}

Result SocketStreamBase::ImportSocket(Socket* socket)
{
    m_socket = socket;
    void* importState = ::operator new(4);
    if (importState != 0)
        memset(importState, 0, 4);
    return 0;
}

}}}
