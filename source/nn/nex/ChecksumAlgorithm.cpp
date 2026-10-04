#include "nn/nex/ChecksumAlgorithm.hpp"

#include <string.h>

namespace nn
{
namespace nex
{

namespace
{

const unsigned int APPEND_AT_END = 0xFFFFFFFFu;

}

ChecksumAlgorithm::ChecksumAlgorithm()
    : m_uiChecksumLength(0)
{
}

unsigned char ChecksumAlgorithm::ReduceChecksumLength(unsigned char uiLength)
{
    m_uiChecksumLength = uiLength;
    if (uiLength == 0)
    {
        uiLength = static_cast<unsigned char>(GetChecksumLength());
        m_uiChecksumLength = uiLength;
    }

    if (GetChecksumLength() < uiLength)
        m_uiChecksumLength = static_cast<unsigned char>(GetChecksumLength());

    return m_uiChecksumLength;
}

void ChecksumAlgorithm::CompressChecksum(Buffer* pChecksum)
{
    unsigned char uiLength = m_uiChecksumLength;
    unsigned int uiFullLength = GetChecksumLength();
    for (unsigned int i = uiLength; i < uiFullLength; ++i)
    {
        unsigned char* pSource = &(*pChecksum)[i];
        unsigned char* pDestination = &(*pChecksum)[m_uiChecksumLength - 1];
        *pDestination = *pDestination ^ *pSource;
    }

    pChecksum->SetLength(m_uiChecksumLength);
}

bool ChecksumAlgorithm::AppendChecksum(Buffer* pBuffer)
{
    if (!IsReady())
        return false;

    if (m_uiChecksumLength == 0)
        m_uiChecksumLength = static_cast<unsigned char>(GetChecksumLength());

    Buffer checksum(GetChecksumLength(), 0);
    ComputeChecksum(*pBuffer, &checksum);
    if (m_uiChecksumLength < GetChecksumLength())
        CompressChecksum(&checksum);

    pBuffer->AppendData(checksum.GetData(), checksum.GetLength(), APPEND_AT_END);
    return true;
}

bool ChecksumAlgorithm::RemoveChecksum(Buffer* pBuffer)
{
    if (!IsReady())
        return false;

    unsigned int uiLength = m_uiChecksumLength;
    if (uiLength == 0)
    {
        uiLength = GetChecksumLength();
        m_uiChecksumLength = static_cast<unsigned char>(uiLength);
    }

    if (uiLength > pBuffer->GetLength())
        return false;

    Buffer received;
    unsigned char uiChecksumLength = m_uiChecksumLength;
    pBuffer->SetLength(pBuffer->GetLength() - uiChecksumLength);
    received.AppendData(pBuffer->GetData() + pBuffer->GetLength(), uiChecksumLength,
        APPEND_AT_END);

    Buffer computed(GetChecksumLength(), 0);
    ComputeChecksum(*pBuffer, &computed);
    if (m_uiChecksumLength < GetChecksumLength())
        CompressChecksum(&computed);

    return received == computed;
}

Key ChecksumAlgorithm::DeriveKey(const Buffer& rBuffer, unsigned int uiIterations)
{
    Buffer current(rBuffer);
    Buffer derived(GetChecksumLength(), 0);
    for (unsigned int uiCount = 0; uiCount < uiIterations; ++uiCount)
    {
        derived.Clear();
        ComputeChecksum(current, &derived);
        current = derived;
    }

    return Key(current.GetData(), current.GetLength());
}

Key ChecksumAlgorithm::DeriveKey(const char* pString, unsigned int uiIterations)
{
    Buffer buffer;
    buffer.AppendData(pString, static_cast<unsigned int>(strlen(pString)), APPEND_AT_END);
    return DeriveKey(buffer, uiIterations);
}

}
}
