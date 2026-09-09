#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_CRYPTO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_CRYPTO_HPP

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace common {

class Crypto
{
public:
    typedef unsigned int Mode;

    class Setting;

    static Result Encrypt(void* destination, const void* source, unsigned int size,
                          void* signature, unsigned int signatureSize,
                          const Setting& setting);
    static Result Decrypt(void* destination, const void* source, unsigned int size,
                          void* signature, unsigned int signatureSize,
                          const Setting& setting);
    static unsigned int GetBlockSize(Mode mode);
};

}}}

#endif
