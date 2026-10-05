#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__TYPE_BUFFER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__TYPE_BUFFER_HPP

namespace nn
{
namespace nex
{

class Buffer;
class Message;

class _Type_buffer
{
public:
    static bool Extract(Message* pMessage, Buffer* pDestination);
};

}
}

#endif
