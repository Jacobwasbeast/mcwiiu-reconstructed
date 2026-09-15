#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_AES_IMPL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_AES_IMPL_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace common {

template <typename Version>
class AesImpl
{
public:
    ~AesImpl();
    void Initialize(const void* key, unsigned int length, bool encrypt);
    void EncryptBlock(void* destination, unsigned int destinationLength,
                      const void* source, unsigned int sourceLength) const;
    void DecryptBlock(void* destination, unsigned int destinationLength,
                      const void* source, unsigned int sourceLength) const;

private:
    uint32_t m_roundKeys[44];
};

}}}

#endif
