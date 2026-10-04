#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/common/common.hpp"


namespace nn { namespace pia { namespace common {


unsigned int Crypto::GetBlockSize(Mode mode)
{
    if (mode == cModeAesGcm)
        return 16;
    return 0;
}

Result Crypto::Encrypt(void* destination, const void* source, unsigned int size,
                       void* signature, unsigned int signatureSize,
                       const Setting& setting)
{
    if (setting.m_mode == cModeNone)
        return 0;
    if (setting.m_mode != cModeAesGcm || setting.m_keyLength != 0x10)
        return 0x10C07;

    AesImpl<XCUiL_2_16> aes;
    GcmMode gcm;
    aes.Initialize(setting.m_key, 0x10, true);
    gcm.Initialize(aes);
    gcm.Reset(setting.m_iv, setting.m_ivLength);
    if ((size & 0x0F) != 0)
        return 0x10C07;

    gcm.EncryptUpdate(destination, size, source, size);
    if (signature != 0 && signatureSize == 0x10)
        gcm.GetMac(signature, 0x10);
    return 0;
}

Result Crypto::Decrypt(void* destination, const void* source, unsigned int size,
                       void* signature, unsigned int signatureSize,
                       const Setting& setting)
{
    if (setting.m_mode == cModeNone)
        return 0;
    if (setting.m_mode != cModeAesGcm || setting.m_keyLength != 0x10)
        return 0x10C07;

    AesImpl<XCUiL_2_16> aes;
    GcmMode gcm;
    aes.Initialize(setting.m_key, 0x10, true);
    gcm.Initialize(aes);
    gcm.Reset(setting.m_iv, setting.m_ivLength);
    if ((size & 0x0F) != 0)
        return 0x10C07;

    gcm.DecryptUpdate(destination, size, source, size);
    if (signature != 0 && signatureSize == 0x10)
    {
        unsigned char mac[0x10];
        gcm.GetMac(mac, 0x10);
        for (unsigned int index = 0; index < 0x10; ++index)
        {
            if (static_cast<const unsigned char*>(signature)[index] != mac[index])
                return 0x2C03;
        }
    }
    return 0;
}

}}}

namespace nn { namespace crypto {

void GenerateRandomBytes(void* destination, unsigned int size);

}}

namespace nn { namespace pia { namespace common { namespace crypto {

void ClearMemory(void* destination, unsigned int size)
{
    unsigned char* bytes = static_cast<unsigned char*>(destination);
    for (int index = 0; index < static_cast<int>(size); ++index)
        bytes[index] = 0;
}

unsigned long long GenerateRandomSeed()
{
    unsigned long long seed = 0;
    nn::crypto::GenerateRandomBytes(&seed, sizeof(seed));
    return seed;
}

void GenerateRandomParameter(void* destination, unsigned int size)
{
    nn::crypto::GenerateRandomBytes(destination, size);
}

}}}}
