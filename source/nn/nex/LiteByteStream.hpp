#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LITEBYTESTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LITEBYTESTREAM_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/LiteBuffer.hpp"

namespace nn
{
namespace nex
{

template <unsigned int Capacity>
class LiteByteStream : public RootObject
{
public:
    LiteByteStream() : m_field0(0), m_buffer(), m_field40(false)
    {
    }

    virtual ~LiteByteStream()
    {
    }

private:
    unsigned int m_field0;
    LiteBuffer<Capacity> m_buffer;
    bool m_field40;
};

}
}

#endif
