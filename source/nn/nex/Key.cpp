#include "nn/nex/Key.hpp"

#include <string.h>

#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

Key::Key()
{
}

Key::Key(const unsigned char* pContent, unsigned int uiLength)
    : m_content(uiLength)
{
    for (unsigned int i = 0; i < uiLength; ++i)
        m_content[i] = pContent[i];
}

Key::Key(unsigned int uiLength)
    : m_content(uiLength)
{
    memset(&m_content[0], 0, uiLength);
}

Key::Key(const String& rString)
{
    unsigned int uiLength = rString.GetLength() >> 1;
    m_content.resize(uiLength);
    const wchar_t* pChars = rString;
    for (unsigned int i = 0; i < uiLength; ++i)
    {
        unsigned int uiHigh = pChars[i * 2];
        unsigned int uiLow = pChars[i * 2 + 1];
        unsigned int uiHighValue = uiHigh - '0' < 10 ? uiHigh - '0' : uiHigh - 'W';
        unsigned int uiLowValue = uiLow - '0' < 10 ? uiLow - '0' : uiLow - 'W';
        m_content[i] = static_cast<unsigned char>((uiHighValue << 4) | uiLowValue);
    }
}

Key::Key(const Key& rKey)
    : RefCountedObject()
    , m_content(rKey.m_content)
{
}

Key::~Key()
{
}

Key& Key::operator=(const Key& rKey)
{
    m_content = rKey.m_content;
    return *this;
}

unsigned int Key::GetLength() const
{
    return static_cast<unsigned int>(m_content.size());
}

const unsigned char* Key::GetContentPtr() const
{
    if (m_content.empty())
        return 0;

    return &m_content[0];
}

unsigned char* Key::GetContentPtr()
{
    if (m_content.empty())
        return 0;

    return &m_content[0];
}

unsigned char* Key::PrepareContentPtr(unsigned int uiLength)
{
    m_content.resize(uiLength);
    return &m_content[0];
}

void Key::ExtractToString(String* pString) const
{
    pString->Reserve(GetLength() * 2 + 1);
    unsigned int i = 0;
    for (; i < GetLength(); ++i)
    {
        wchar_t* pChars = *pString;
        unsigned char uiByte = m_content[i];
        unsigned int uiHigh = uiByte >> 4;
        unsigned int uiLow = uiByte & 0xF;
        pChars[i * 2] = static_cast<wchar_t>(uiHigh + (uiHigh < 10 ? '0' : 'W'));
        pChars[i * 2 + 1] = static_cast<wchar_t>(uiLow + (uiLow < 10 ? '0' : 'W'));
    }
    wchar_t* pTerminator = *pString;
    pTerminator[i * 2] = 0;
}

String Key::ToString() const
{
    String string;
    ExtractToString(&string);
    return string;
}

void Key::Trace(uint64_t uiFilter) const
{
    if (GetLength() == 0)
        return;

    String string;
    ExtractToString(&string);
}

}
}
