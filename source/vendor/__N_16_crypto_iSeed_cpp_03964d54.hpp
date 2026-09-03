#ifndef SOURCE_VENDOR_N_16_CRYPTO_ISEED_CPP_03964D54_HPP
#define SOURCE_VENDOR_N_16_CRYPTO_ISEED_CPP_03964D54_HPP

#include <stdint.h>

class __N_16_crypto_iSeed_cpp_03964d54
{
public:
    static uint64_t rdtsc();
    static uint64_t GetTicks();
    static uint64_t hash64shift(unsigned long long value);
    static void Stir();
    static uint32_t AddToSeed(unsigned char *data, unsigned int length);
};

#endif
