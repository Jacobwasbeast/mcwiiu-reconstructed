#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LITEBUFFER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LITEBUFFER_HPP

#include <cstring>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/SystemError.hpp"

namespace nn
{
namespace nex
{

template <unsigned int Capacity>
class LiteBuffer : public RootObject
{
public:
    LiteBuffer() : m_length(0)
    {
    }

    virtual ~LiteBuffer()
    {
    }

    bool AppendData(const void* data, unsigned int size, unsigned int offset)
    {
        if (size == 0)
        {
            return true;
        }

        if (offset == 0xFFFFFFFFu)
        {
            offset = m_length;
        }

        unsigned int length = m_length;
        if (offset > length || offset + size > length)
        {
            length = offset + size;
        }

        if (length > Capacity)
        {
            static wchar_t file[] = L"D:\\home\\jenkins\\workspace\\Master\\Pack\\d4f027b8\\.\\OnlineCore/src/Core/LiteBuffer.h";
            SystemError::SignalError(file, 79, 0xE0000019u, 0);
            return false;
        }

        m_length = length;
        std::memcpy(m_data + offset, data, size);
        return true;
    }

private:
    unsigned int m_length;
    unsigned char m_data[Capacity];
};

}
}

#endif
