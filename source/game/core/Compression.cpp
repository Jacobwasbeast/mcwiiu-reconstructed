#include "game/core/Compression.hpp"

#include <string.h>
#include <zlib.h>

enum _XMEMCODEC_TYPE
{
    XMEMCODEC_DEFAULT = 0,
    XMEMCODEC_LZX = 1
};

struct XMEMCODEC_PARAMETERS_LZX
{
    unsigned int Flags;
    unsigned int WindowSize;
    unsigned int CompressionPartitionSize;
};

int XMemCreateCompressionContext(_XMEMCODEC_TYPE codecType, const void* pCodecParams, unsigned int flags, void** ppContext);
int XMemCreateDecompressionContext(_XMEMCODEC_TYPE codecType, const void* pCodecParams, unsigned int flags, void** ppContext);
void XMemDestroyCompressionContext(void* pContext);
void XMemDestroyDecompressionContext(void* pContext);

unsigned int TlsAlloc();
int TlsSetValue(unsigned int index, void* pValue);
void* TlsGetValue(unsigned int index);

void PIXBeginNamedEvent(float colour, const char* pName, ...);
void PIXEndNamedEvent();

static unsigned int g_compressionTlsIndex = 0;
static Compression::ThreadStorage* g_pDefaultThreadStorage = NULL;

Compression::Compression()
{
    XMEMCODEC_PARAMETERS_LZX codecParams;

    codecParams.Flags = 0;
    codecParams.WindowSize = 128 * 1024;
    codecParams.CompressionPartitionSize = 128 * 1024;

    m_pDecompressBuffer = NULL;

    XMemCreateCompressionContext(XMEMCODEC_LZX, &codecParams, 0, &m_pCompressionContext);
    XMemCreateDecompressionContext(XMEMCODEC_LZX, &codecParams, 0, &m_pDecompressionContext);

    m_compressionType = eCompressionTypes_Zlib;
    m_decompressionType = eCompressionTypes_Zlib;

    InitializeCriticalSection(&m_compressLock);
    InitializeCriticalSection(&m_decompressLock);
}

Compression::~Compression()
{
    XMemDestroyCompressionContext(m_pCompressionContext);
    XMemDestroyDecompressionContext(m_pDecompressionContext);

    DeleteCriticalSection(&m_compressLock);
    DeleteCriticalSection(&m_decompressLock);

    delete m_pDecompressBuffer;
}

Compression::ThreadStorage::ThreadStorage()
{
    m_pCompression = new Compression();
}

Compression::ThreadStorage::~ThreadStorage()
{
    delete m_pCompression;
}

void Compression::CreateNewThreadStorage()
{
    ThreadStorage* pStorage = new ThreadStorage();

    if (g_pDefaultThreadStorage == NULL)
    {
        g_compressionTlsIndex = TlsAlloc();
        g_pDefaultThreadStorage = pStorage;
    }

    TlsSetValue(g_compressionTlsIndex, pStorage);
}

void Compression::UseDefaultThreadStorage()
{
    TlsSetValue(g_compressionTlsIndex, g_pDefaultThreadStorage);
}

void Compression::ReleaseThreadStorage()
{
    ThreadStorage* pStorage = (ThreadStorage*)TlsGetValue(g_compressionTlsIndex);

    if (pStorage != g_pDefaultThreadStorage)
    {
        delete pStorage;
    }
}

Compression* Compression::getCompression()
{
    ThreadStorage* pStorage = (ThreadStorage*)TlsGetValue(g_compressionTlsIndex);

    return pStorage->m_pCompression;
}

int Compression::internalCompressRle(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    unsigned char* pOutput = (unsigned char*)pDest;
    unsigned char* pOutputLimit = (unsigned char*)pDest + *pDestSize - 3;
    unsigned char* pInput = (unsigned char*)pSource;
    unsigned char* pInputEnd = (unsigned char*)pSource + sourceSize;

    PIXBeginNamedEvent(0.0f, "RLE compression");

    do
    {
        unsigned char value = *pInput;
        unsigned int runLength = 1;

        ++pInput;

        while (pInput != pInputEnd && *pInput == value && runLength < 256)
        {
            ++pInput;
            ++runLength;
        }

        if (runLength < 4)
        {
            if (value == 0xFF)
            {
                pOutput[0] = 0xFF;
                pOutput[1] = (unsigned char)(runLength - 1);
                pOutput += 2;
            }
            else
            {
                memset(pOutput, value, runLength);
                pOutput += runLength;
            }
        }
        else
        {
            pOutput[0] = 0xFF;
            pOutput[1] = (unsigned char)(runLength - 1);
            pOutput[2] = value;
            pOutput += 3;
        }
    }
    while (pInput != pInputEnd && pOutput < pOutputLimit);

    PIXEndNamedEvent();

    *pDestSize = (unsigned int)(pOutput - (unsigned char*)pDest);

    if (pInput != pInputEnd)
    {
        return 0x80CC0001;
    }

    return 0;
}

int Compression::internalDecompressRle(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    unsigned char* pOutput = (unsigned char*)pDest;
    unsigned char* pInput = (unsigned char*)pSource;
    unsigned char* pInputEnd = (unsigned char*)pSource + sourceSize;

    while (pInput != pInputEnd)
    {
        unsigned char value = *pInput;

        ++pInput;

        if (value != 0xFF)
        {
            *pOutput = value;
            ++pOutput;
        }
        else
        {
            unsigned int runLength = *pInput;
            unsigned char runValue = 0xFF;

            ++pInput;

            if (runLength > 2)
            {
                runValue = *pInput;
                ++pInput;
            }

            ++runLength;

            memset(pOutput, runValue, runLength);
            pOutput += runLength;
        }
    }

    *pDestSize = (unsigned int)(pOutput - (unsigned char*)pDest);

    return 0;
}

int Compression::Compress(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    uLong compressedSize = *pDestSize;

    if (pDest == NULL)
    {
        *pDestSize = compressBound(sourceSize);

        return 0;
    }

    int result = compress((Bytef*)pDest, &compressedSize, (const Bytef*)pSource, sourceSize);

    *pDestSize = compressedSize;

    if (result != 0)
    {
        return -1;
    }

    return 0;
}

int Compression::DecompressWithType(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    if (m_decompressionType < eCompressionTypes_LZXRLE)
    {
        memcpy(pDest, pSource, sourceSize);
        *pDestSize = sourceSize;

        return 0;
    }

    return -1;
}

int Compression::Decompress(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    if (m_decompressionType != m_compressionType)
    {
        return DecompressWithType(pDest, pDestSize, pSource, sourceSize);
    }

    EnterCriticalSection(&m_decompressLock);

    uLong decompressedSize = *pDestSize;

    int result = uncompress((Bytef*)pDest, &decompressedSize, (const Bytef*)pSource, sourceSize);

    *pDestSize = decompressedSize;

    LeaveCriticalSection(&m_decompressLock);

    if (result != 0)
    {
        return -1;
    }

    return 0;
}

int Compression::CompressLZXRLE(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize, unsigned int* pRleSize)
{
    EnterCriticalSection(&m_compressLock);

    unsigned int rleSize = sizeof(m_rleBuffer);

    int result = internalCompressRle(m_rleBuffer, &rleSize, pSource, sourceSize);

    if (pRleSize != NULL)
    {
        *pRleSize = rleSize;
    }

    if (result == 0)
    {
        PIXBeginNamedEvent(0.0f, "Secondary compression");
        result = Compress(pDest, pDestSize, m_rleBuffer, rleSize);
        PIXEndNamedEvent();
    }

    LeaveCriticalSection(&m_compressLock);

    return result;
}

int Compression::DecompressLZXRLE(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    unsigned char* pScratchBuffer = NULL;

    EnterCriticalSection(&m_decompressLock);

    unsigned int decompressedSize = *pDestSize;

    if (decompressedSize > 0x32000)
    {
        pScratchBuffer = new unsigned char[decompressedSize];

        Decompress(pScratchBuffer, &decompressedSize, pSource, sourceSize);
        internalDecompressRle(pDest, pDestSize, pScratchBuffer, decompressedSize);
    }
    else
    {
        decompressedSize = 0x32000;

        if (m_pDecompressBuffer == NULL)
        {
            m_pDecompressBuffer = new unsigned char[0x32000];
        }

        Decompress(m_pDecompressBuffer, &decompressedSize, pSource, sourceSize);
        internalDecompressRle(pDest, pDestSize, m_pDecompressBuffer, decompressedSize);
    }

    delete[] pScratchBuffer;

    LeaveCriticalSection(&m_decompressLock);

    return 0;
}

int Compression::CompressRLE(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    EnterCriticalSection(&m_compressLock);

    unsigned int rleSize = sizeof(m_rleBuffer);

    int result = internalCompressRle(m_rleBuffer, &rleSize, pSource, sourceSize);

    if (rleSize > *pDestSize)
    {
        result = 0x80CC0002;
    }
    else if (result == 0)
    {
        *pDestSize = rleSize;
        memcpy(pDest, m_rleBuffer, rleSize);
    }

    LeaveCriticalSection(&m_compressLock);

    return result;
}

int Compression::DecompressRLE(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    EnterCriticalSection(&m_decompressLock);

    internalDecompressRle(pDest, pDestSize, pSource, sourceSize);

    LeaveCriticalSection(&m_decompressLock);

    return 0;
}

void Compression::VitaVirtualDecompress(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    unsigned char* pOutput = (unsigned char*)pDest;
    unsigned char* pInput = (unsigned char*)pSource;
    unsigned int outputSize = 0;
    unsigned int inputOffset = 0;

    while (inputOffset != sourceSize)
    {
        unsigned char value = pInput[inputOffset];

        if (value != 0)
        {
            pOutput[outputSize] = value;
            ++outputSize;
            ++inputOffset;
        }
        else
        {
            unsigned char runLengthByte = pInput[inputOffset + 1];

            inputOffset += 2;

            if (runLengthByte != 0)
            {
                unsigned int blockCount = runLengthByte >> 3;
                unsigned int remainder = runLengthByte & 7;
                unsigned int runLength = blockCount * 8 + remainder;

                memset(pOutput + outputSize, 0, runLength);
                outputSize += runLength;
            }
        }
    }

    *pDestSize = outputSize;
}

void Compression::SetDecompressionType(ESavePlatform platform)
{
    switch (platform)
    {
    case eSavePlatform_X360:
        getCompression()->m_decompressionType = eCompressionTypes_LZXRLE;
        break;

    case eSavePlatform_PS3:
        getCompression()->m_decompressionType = eCompressionTypes_ZlibPS3;
        break;

    case eSavePlatform_WIIU:
    case eSavePlatform_XB1:
    case eSavePlatform_NX:
    case eSavePlatform_PS4:
    case eSavePlatform_WIN:
    case eSavePlatform_PSV:
        getCompression()->m_decompressionType = eCompressionTypes_Zlib;
        break;

    default:
        break;
    }
}
