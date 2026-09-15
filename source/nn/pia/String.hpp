#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_STRING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_STRING_HPP

#include <cstdarg>
#include <cstdint>

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace common {

class String
{
public:
    enum EncodingType { EncodingType_Ascii = 1, EncodingType_Utf16 = 2 };

    String(const char* text);
    String(const wchar_t* text);
    String(const String& other);
    virtual ~String();
    virtual Result Copy(const String& other);
    virtual void Clear();
    virtual void Trace(unsigned long long flag) const;
    virtual Result SetString(const void* text, unsigned int length, EncodingType encoding);
    int Format(const char* format, ...);
    int FormatV(const char* format, va_list args);
    unsigned int CalcBytes(const void* text, EncodingType encoding);
    const char* GetCharArray() const;
    const void* GetBuffer() const;
    unsigned int GetBytes() const;
    EncodingType GetEncodingType() const;
    unsigned int GetStringLength() const;
    bool IsSameString(const String& other) const;
    bool operator==(const String& other) const;
    String& operator=(const String& other)
    {
        Copy(other);
        return *this;
    }

protected:
    String()
        : m_text(0), m_bufferSize(0), m_bytes(0), m_encoding()
    {
    }

    const void* m_text;
    unsigned int m_bufferSize;
    unsigned int m_bytes;
    EncodingType m_encoding;
};

template <unsigned int LengthT>
class FixedString : public String
{
public:
    FixedString();
    virtual ~FixedString();
    virtual void Clear();
    virtual Result SetString(const void* text, unsigned int length, EncodingType encoding);

    FixedString& operator=(const String& other)
    {
        Copy(other);
        return *this;
    }
    FixedString& operator=(const FixedString& other)
    {
        Copy(other);
        return *this;
    }

private:
    char m_buffer[LengthT * 4 + 1];
};

template <unsigned int LengthT>
FixedString<LengthT>::FixedString()
    : String()
{
    m_bufferSize = LengthT * 4;
    for (unsigned int index = 0; index < LengthT * 4 + 1; ++index)
        m_buffer[index] = 0;
    m_bytes = 0;
    m_text = m_buffer;
    m_encoding = EncodingType();
}

template <unsigned int LengthT>
FixedString<LengthT>::~FixedString()
{
}

template <unsigned int LengthT>
void FixedString<LengthT>::Clear()
{
    for (unsigned int index = 0; index < LengthT * 4 + 1; ++index)
        m_buffer[index] = 0;
    m_text = m_buffer;
    m_bytes = 0;
    m_encoding = EncodingType();
}

template <unsigned int LengthT>
Result FixedString<LengthT>::SetString(const void* text, unsigned int length, EncodingType encoding)
{
    if (text == 0)
        return 0x10c07;

    unsigned int bufferSize = m_bufferSize;
    if (bufferSize < length)
        return 0x10c07;

    if (encoding == EncodingType_Utf16)
    {
        if (((bufferSize >> 1) & 0x7ffffffe) < length)
            return 0x10c07;
    }
    else if (encoding == EncodingType_Ascii)
    {
        const unsigned char* value = static_cast<const unsigned char*>(text);
        unsigned int characters = 0;
        while (*value != 0)
        {
            unsigned int code = *value;
            if (code - 1 < 0x7f)
            {
                value += 1;
                ++characters;
            }
            else if (code - 0xc2 < 0x1e)
            {
                value += 2;
                ++characters;
            }
            else if (code - 0xe0 < 0x10)
            {
                value += 3;
                ++characters;
            }
            else if (code - 0xf0 < 8)
            {
                value += 4;
                ++characters;
            }
            else if (code - 0xf8 < 4)
            {
                value += 5;
                ++characters;
            }
            else if (code - 0xfc < 2)
            {
                value += 6;
                ++characters;
            }
            else
            {
                value += 1;
            }
        }
        if ((bufferSize >> 2) < characters)
            return 0x10c07;
    }
    else
    {
        return 0x10c07;
    }

    unsigned int index = 0;
    while (index < m_bufferSize)
    {
        m_buffer[index] = 0;
        ++index;
    }

    index = 0;
    if (encoding == EncodingType_Ascii)
    {
        const char* value = static_cast<const char*>(text);
        while (*value != '\0' && index < m_bufferSize)
        {
            m_buffer[index] = *value;
            ++value;
            ++index;
        }
        m_buffer[index] = '\0';
        m_bytes = index;
        m_encoding = EncodingType_Ascii;
        m_text = m_buffer;
    }
    else
    {
        if (encoding == EncodingType_Utf16)
        {
            const std::uint16_t* value = static_cast<const std::uint16_t*>(text);
            std::uint16_t* buffer = reinterpret_cast<std::uint16_t*>(m_buffer);
            while (*value != 0 && index < ((m_bufferSize >> 1) & 0x7ffffffe))
            {
                buffer[index >> 1] = *value;
                ++value;
                index += 2;
            }
            m_buffer[index] = 0;
            m_buffer[index + 1] = 0;
        }
        m_encoding = encoding;
        m_text = m_buffer;
        m_bytes = index;
    }
    return 0;
}

}}}

#endif
