#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CHECKSUMALGORITHM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CHECKSUMALGORITHM_HPP

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/PolymorphicRootObject.hpp"
#include "nn/nex/SignatureBytes.hpp"

namespace nn
{
namespace nex
{

class ChecksumAlgorithm : public PolymorphicRootObject
{
public:
    ChecksumAlgorithm();
    virtual ~ChecksumAlgorithm() {}

    virtual bool ComputeChecksum(const Buffer& rBuffer, Buffer* pChecksum) = 0;
    virtual bool ComputeChecksum(const unsigned char** ppData, const unsigned int* pLengths,
        int iCount, SignatureBytes& rSignature) = 0;
    virtual bool IsReady() const
    {
        return true;
    }
    virtual unsigned int ComputeChecksumForTransport(const unsigned char* pData,
        unsigned int uiLength)
    {
        return ComputeChecksumForTransportArray(&pData, &uiLength, 1);
    }
    virtual unsigned int ComputeChecksumForTransportArray(const unsigned char** ppData,
        const unsigned int* pLengths, int iCount) = 0;
    virtual unsigned int GetChecksumLength() = 0;

    unsigned char ReduceChecksumLength(unsigned char uiLength);
    void CompressChecksum(Buffer* pChecksum);
    bool AppendChecksum(Buffer* pBuffer);
    bool RemoveChecksum(Buffer* pBuffer);
    Key DeriveKey(const Buffer& rBuffer, unsigned int uiIterations);
    Key DeriveKey(const char* pString, unsigned int uiIterations);

protected:
    unsigned char m_uiChecksumLength;
};

}
}

#endif
