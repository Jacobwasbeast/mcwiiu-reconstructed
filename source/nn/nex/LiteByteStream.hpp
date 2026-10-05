#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LITEBYTESTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LITEBYTESTREAM_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/LiteBuffer.hpp"

namespace nn
{
namespace nex
{

template <int Capacity>
class LiteByteStream : public RootObject
{
public:
    LiteByteStream() : m_uiPosition(0), m_buffer(), m_bError(false)
    {
    }

    virtual ~LiteByteStream()
    {
    }

    LiteBuffer<Capacity>& GetBuffer()
    {
        return m_buffer;
    }

    const LiteBuffer<Capacity>& GetBuffer() const
    {
        return m_buffer;
    }

    void Clear()
    {
        m_buffer.Clear();
        m_bError = false;
        m_uiPosition = 0;
    }

private:
    unsigned int m_uiPosition;
    LiteBuffer<Capacity> m_buffer;
    bool m_bError;
};

}
}

#endif
