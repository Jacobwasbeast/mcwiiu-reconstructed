#include "nn/nex/ByteStream.hpp"

#include "nn/nex/StringStream.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int APPEND_AT_END = 0xFFFFFFFFu;

}

void SetUnalignedNexNetworkOrder64(uint64_t uiValue, unsigned char* pDestination)
{
    const uint32_t uiLow = static_cast<uint32_t>(uiValue);
    const uint32_t uiHigh = static_cast<uint32_t>(uiValue >> 32);

    pDestination[0] = static_cast<unsigned char>(uiLow);
    pDestination[1] = static_cast<unsigned char>(uiLow >> 8);
    pDestination[2] = static_cast<unsigned char>(uiLow >> 16);
    pDestination[3] = static_cast<unsigned char>(uiLow >> 24);
    pDestination[4] = static_cast<unsigned char>(uiHigh);
    pDestination[5] = static_cast<unsigned char>(uiHigh >> 8);
    pDestination[6] = static_cast<unsigned char>(uiHigh >> 16);
    pDestination[7] = static_cast<unsigned char>(uiHigh >> 24);
}

ByteStream::ByteStream()
{
    m_pBuffer = new Buffer();
    Init();
}

ByteStream::ByteStream(unsigned int uiSize)
{
    m_pBuffer = new Buffer(uiSize, 0);
    Init();
}

ByteStream::ByteStream(Buffer* pBuffer)
{
    m_pBuffer = pBuffer;
    m_pBuffer->AddRef();
    Init();
}

ByteStream::~ByteStream()
{
    m_pBuffer->ReleaseRef();
    m_pBuffer = 0;
}

void ByteStream::Init()
{
    m_bError = false;
    m_uiPosition = 0;
}

void ByteStream::Clear()
{
    m_pBuffer->Clear();
    Init();
}

bool ByteStream::SetLength(unsigned int uiLength)
{
    if (m_pBuffer->GetRealSize() - m_pBuffer->GetHeadShiftSize() < uiLength)
    {
        return false;
    }

    m_pBuffer->SetLength(uiLength);
    return true;
}

bool ByteStream::ValidateBufferLimit(unsigned int uiSize)
{
    if (uiSize <= m_pBuffer->GetLength() - m_uiPosition)
    {
        return true;
    }

    m_bError = true;
    return false;
}

void ByteStream::AppendRaw(const unsigned char* pData, unsigned int uiSize)
{
    m_pBuffer->AppendData(pData, uiSize, APPEND_AT_END);
}

bool ByteStream::Append(const unsigned char* pData, unsigned int uiSize, unsigned int uiCount)
{
    for (unsigned int uiIndex = 0; uiIndex < uiCount; ++uiIndex)
    {
        for (int iByte = static_cast<int>(uiSize) - 1; iByte >= 0; --iByte)
        {
            AppendRaw(pData + uiIndex * uiSize + iByte, 1);
        }
    }

    return true;
}

bool ByteStream::Append(const Time* pTime)
{
    unsigned char aBytes[8];

    SetUnalignedNexNetworkOrder64(*pTime, aBytes);
    m_pBuffer->AppendData(aBytes, sizeof(aBytes), APPEND_AT_END);
    return true;
}

bool ByteStream::ExtractRaw(unsigned char* pData, unsigned int uiSize)
{
    const bool bComplete = ValidateBufferLimit(uiSize);

    if (!bComplete)
    {
        uiSize = m_pBuffer->GetLength() - m_uiPosition;
    }

    if (uiSize != 0)
    {
        m_pBuffer->CopyContent(pData, uiSize, m_uiPosition);
        m_uiPosition += uiSize;
    }

    return bComplete;
}

void ByteStream::Extract(unsigned char* pData, unsigned int uiSize, unsigned int uiCount)
{
    if (uiSize == 1)
    {
        ExtractRaw(pData, uiCount);
        return;
    }

    bool bComplete = true;

    for (unsigned int uiIndex = 0; uiIndex < uiCount && bComplete; ++uiIndex)
    {
        for (int iByte = static_cast<int>(uiSize) - 1; iByte >= 0 && bComplete; --iByte)
        {
            bComplete = ExtractRaw(pData + uiIndex * uiSize + iByte, 1);
        }
    }
}

void ByteStream::Extract(Time* pTime)
{
    uint64_t uiValue = 0;

    if (ExtractPrimitive(&uiValue))
    {
        *pTime = Time(uiValue);
    }
}

ByteStream& ByteStream::operator<<(const Buffer& rBuffer)
{
    AppendRaw(rBuffer.GetData(), rBuffer.GetLength());
    return *this;
}

ByteStream& ByteStream::operator<<(const bool& rValue)
{
    const unsigned char uiValue = static_cast<unsigned char>(rValue);

    Append(&uiValue, 1, 1);
    return *this;
}

ByteStream& ByteStream::operator>>(bool& rValue)
{
    signed char cValue = 0;

    ExtractPrimitive(&cValue);
    rValue = cValue != 0;
    return *this;
}

void ByteStream::TraceBytes(uint64_t uiFilter, unsigned char* pData, unsigned int uiSize)
{
    StringStream stream;

    zerohex(stream);

    for (unsigned int uiIndex = 0; uiIndex < uiSize; ++uiIndex)
    {
        if (uiIndex != 0 && ((uiIndex + 1) & 7) == 0)
        {
            stream.StreamNumber(pData[uiIndex]);
            stream.Clear();
        }
        else
        {
            stream.StreamNumber(pData[uiIndex]) << L" ";
        }
    }
}

void ByteStream::Trace(uint64_t uiFilter, bool bDumpContents)
{
    if (m_pBuffer == 0 || !bDumpContents)
    {
        return;
    }

    TraceBytes(uiFilter, m_pBuffer->GetData(), m_pBuffer->GetLength());
}

}
}
