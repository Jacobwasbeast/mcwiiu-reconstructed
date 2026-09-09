#include "nn/nex/StringStream.hpp"

#include <cstring>
#include <cwchar>

#include "nn/nex/SpecialArray.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int NUMBER_FIELD_LENGTH = 0x20;

}

StringStream::StringStream()
{
    m_pBuffer = m_aInlineBuffer;
    m_uiCapacity = INLINE_BUFFER_LENGTH;
    m_bTextualBool = true;
    m_bHex = false;
    m_bAddBase = true;
    m_aInlineBuffer[0] = 0;
    m_pCursor = m_aInlineBuffer;
}

StringStream::~StringStream()
{
    FreeBuffer();
}

int StringStream::GetLength() const
{
    return static_cast<int>(m_pCursor - m_pBuffer);
}

void StringStream::FreeBuffer(wchar_t* pBuffer)
{
    if (pBuffer == m_aInlineBuffer)
    {
        return;
    }

    SpecialDeleteArray(pBuffer);
}

void StringStream::FreeBuffer()
{
    FreeBuffer(m_pBuffer);
}

void StringStream::ResizeBuffer(unsigned int uiCapacity)
{
    const int iLength = GetLength();
    wchar_t* pOldBuffer = m_pBuffer;
    wchar_t* pNewBuffer = SpecialNewArray<wchar_t>(uiCapacity, L"", 0);

    m_uiCapacity = uiCapacity;
    m_pBuffer = pNewBuffer;

    std::memcpy(pNewBuffer, pOldBuffer, iLength * sizeof(wchar_t) + sizeof(wchar_t));

    m_pCursor = m_pBuffer + iLength;

    FreeBuffer(pOldBuffer);
}

void StringStream::TestFreeRoom(unsigned int uiCount)
{
    const unsigned int uiRequired = static_cast<unsigned int>(GetLength()) + uiCount;
    unsigned int uiCapacity = m_uiCapacity;

    if (uiCapacity < uiRequired)
    {
        while (uiCapacity < uiRequired)
        {
            uiCapacity = uiCapacity + (uiCapacity >> 1);
        }

        ResizeBuffer(uiCapacity);
    }
}

void StringStream::AddBaseIfRequired()
{
    if (m_bAddBase)
    {
        std::wcscat(m_pCursor, L"0x");
        m_pCursor += 2;
    }
}

void StringStream::Clear()
{
    FreeBuffer();

    m_pBuffer = m_aInlineBuffer;
    m_pCursor = m_aInlineBuffer;
    m_aInlineBuffer[0] = 0;
    m_uiCapacity = INLINE_BUFFER_LENGTH;
}

StringStream& StringStream::operator<<(const wchar_t* pString)
{
    if (pString == 0)
    {
        return *this << L"(null)";
    }

    const unsigned int uiLength = static_cast<unsigned int>(std::wcslen(pString));

    TestFreeRoom(uiLength + 1);
    std::wcscpy(m_pCursor, pString);
    m_pCursor += uiLength;
    return *this;
}

StringStream& StringStream::operator<<(const void* pPointer)
{
    TestFreeRoom(NUMBER_FIELD_LENGTH);
    AddBaseIfRequired();

    wchar_t* pCursor = m_pCursor;

    m_pCursor = pCursor + std::swprintf(pCursor, NUMBER_FIELD_LENGTH, L"%llx",
        static_cast<unsigned long long>(reinterpret_cast<unsigned int>(pPointer)));
    return *this;
}

StringStream& StringStream::operator<<(bool bValue)
{
    if (!m_bTextualBool)
    {
        return StreamNumber(static_cast<int>(bValue));
    }

    if (bValue)
    {
        return *this << L"true";
    }

    return *this << L"false";
}

StringStream& StringStream::operator<<(unsigned long long uiValue)
{
    TestFreeRoom(NUMBER_FIELD_LENGTH);

    const wchar_t* pFormat;

    if (!m_bHex)
    {
        pFormat = L"%llu";
    }
    else
    {
        AddBaseIfRequired();

        if (m_bZeroPadded)
        {
            wchar_t* pPadded = m_pCursor;

            m_pCursor = pPadded
                + std::swprintf(pPadded, NUMBER_FIELD_LENGTH, L"%016llx", uiValue);
            return *this;
        }

        pFormat = L"%llx";
    }

    wchar_t* pCursor = m_pCursor;

    m_pCursor = pCursor + std::swprintf(pCursor, NUMBER_FIELD_LENGTH, pFormat, uiValue);
    return *this;
}

StringStream& StringStream::StreamNumber(int iValue)
{
    TestFreeRoom(NUMBER_FIELD_LENGTH);

    const wchar_t* pFormat;

    if (!m_bHex)
    {
        pFormat = L"%d";
    }
    else
    {
        AddBaseIfRequired();

        if (m_bZeroPadded)
        {
            wchar_t* pPadded = m_pCursor;

            m_pCursor = pPadded + std::swprintf(pPadded, NUMBER_FIELD_LENGTH, L"%08x", iValue);
            return *this;
        }

        pFormat = L"%x";
    }

    wchar_t* pCursor = m_pCursor;

    m_pCursor = pCursor + std::swprintf(pCursor, NUMBER_FIELD_LENGTH, pFormat, iValue);
    return *this;
}

StringStream& StringStream::StreamNumber(unsigned int uiValue)
{
    TestFreeRoom(NUMBER_FIELD_LENGTH);

    const wchar_t* pFormat;

    if (!m_bHex)
    {
        pFormat = L"%u";
    }
    else
    {
        AddBaseIfRequired();

        if (m_bZeroPadded)
        {
            wchar_t* pPadded = m_pCursor;

            m_pCursor = pPadded + std::swprintf(pPadded, NUMBER_FIELD_LENGTH, L"%08x", uiValue);
            return *this;
        }

        pFormat = L"%x";
    }

    wchar_t* pCursor = m_pCursor;

    m_pCursor = pCursor + std::swprintf(pCursor, NUMBER_FIELD_LENGTH, pFormat, uiValue);
    return *this;
}

StringStream& StringStream::StreamNumber(unsigned char uiValue)
{
    TestFreeRoom(NUMBER_FIELD_LENGTH);

    const wchar_t* pFormat;

    if (!m_bHex)
    {
        pFormat = L"%u";
    }
    else
    {
        AddBaseIfRequired();

        if (m_bZeroPadded)
        {
            wchar_t* pPadded = m_pCursor;

            m_pCursor = pPadded + std::swprintf(pPadded, NUMBER_FIELD_LENGTH, L"%02x", uiValue);
            return *this;
        }

        pFormat = L"%x";
    }

    wchar_t* pCursor = m_pCursor;

    m_pCursor = pCursor + std::swprintf(pCursor, NUMBER_FIELD_LENGTH, pFormat, uiValue);
    return *this;
}

void zerohex(StringStream& rStream)
{
    rStream.m_bHex = true;
    rStream.m_bZeroPadded = true;
}

}
}
