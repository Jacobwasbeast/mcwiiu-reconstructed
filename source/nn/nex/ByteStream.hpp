#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_BYTESTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_BYTESTREAM_HPP

#include <stdint.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Time;

void SetUnalignedNexNetworkOrder64(uint64_t uiValue, unsigned char* pDestination);

class ByteStream : public RootObject
{
public:
    ByteStream();
    explicit ByteStream(unsigned int uiSize);
    explicit ByteStream(Buffer* pBuffer);
    virtual ~ByteStream();

    void Init();
    void Clear();

    bool SetLength(unsigned int uiLength);
    bool ValidateBufferLimit(unsigned int uiSize);

    void AppendRaw(const unsigned char* pData, unsigned int uiSize);
    bool Append(const unsigned char* pData, unsigned int uiSize, unsigned int uiCount);
    bool Append(const Time* pTime);
    bool AppendString(const wchar_t* pString, unsigned int uiSize, unsigned int uiCount);
    bool AppendString(const wchar_t* pString);
    bool AppendString(const wchar_t* pString, unsigned int uiLength);

    bool ExtractRaw(unsigned char* pData, unsigned int uiSize);
    void Extract(unsigned char* pData, unsigned int uiSize, unsigned int uiCount);
    void Extract(Time* pTime);
    bool ExtractString(wchar_t* pString, unsigned short uiLength);

    bool CanAppend(unsigned int uiSize) const;
    bool CanExtract(unsigned int uiSize) const;

    bool operator==(const ByteStream& rStream) const;
    ByteStream& operator<<(const Buffer& rBuffer);
    ByteStream& operator<<(const bool& rValue);
    ByteStream& operator>>(Buffer& rBuffer);
    ByteStream& operator>>(ByteStream& rStream);
    ByteStream& operator>>(bool& rValue);

    static void TraceBytes(uint64_t uiFilter, unsigned char* pData, unsigned int uiSize);
    void Trace(uint64_t uiFilter, bool bDumpContents);

    template <typename T>
    bool ExtractPrimitive(T* pValue)
    {
        if (!ValidateBufferLimit(sizeof(T)))
        {
            *pValue = 0;
            return false;
        }

        const unsigned char* pSource = m_pBuffer->GetData() + m_uiPosition;
        unsigned char* pDestination = reinterpret_cast<unsigned char*>(pValue);

        for (unsigned int i = 0; i < sizeof(T); ++i)
        {
            pDestination[sizeof(T) - 1 - i] = pSource[i];
        }

        m_uiPosition += sizeof(T);
        return true;
    }

    bool HasError() const
    {
        return m_bError;
    }

    void SetError()
    {
        m_bError = true;
    }

    unsigned int GetPosition() const
    {
        return m_uiPosition;
    }

    void SetPosition(unsigned int uiPosition)
    {
        m_uiPosition = uiPosition;
    }

    unsigned int GetLength() const
    {
        return m_pBuffer->GetLength();
    }

protected:
    bool m_bError;
    Buffer* m_pBuffer;
    unsigned int m_uiPosition;
};

}
}

#endif
