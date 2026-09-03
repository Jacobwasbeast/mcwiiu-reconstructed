#include "vendor/__N_16_crypto_iSeed_cpp_03964d54.hpp"

#include <time.h>

static unsigned char g_seedInitialized;
static uint32_t g_seed0;
static uint32_t g_seed1;
static uint32_t g_seed2;
static uint32_t g_seed3;

uint64_t __N_16_crypto_iSeed_cpp_03964d54::rdtsc()
{
    return static_cast<uint64_t>(clock());
}

uint64_t __N_16_crypto_iSeed_cpp_03964d54::GetTicks()
{
    return static_cast<uint64_t>(clock());
}

uint64_t __N_16_crypto_iSeed_cpp_03964d54::hash64shift(unsigned long long value)
{
    value = (~value) + (value << 21);
    value ^= value >> 24;
    value = value + (value << 3) + (value << 8);
    value ^= value >> 14;
    value = value + (value << 2) + (value << 4);
    value ^= value >> 28;
    value = value + (value << 31);
    return static_cast<uint64_t>(value);
}

void __N_16_crypto_iSeed_cpp_03964d54::Stir()
{
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    uint32_t e;
    uint64_t mixed;

    if (g_seedInitialized == 0)
    {
        g_seedInitialized = 1;
        g_seed0 = 0x9e3779b1U;
        g_seed1 = 0x9e3779b9U;
        g_seed2 = 0x9e3779b9U;
        g_seed3 = 0x9e3779b1U;
    }

    a = (g_seed0 - g_seed1) - g_seed2;
    a ^= g_seed2 >> 13;
    b = (g_seed1 - g_seed2) - a;
    b ^= a << 8;
    c = (g_seed2 - a) - b;
    c ^= b >> 13;
    a = (a - b) - c;
    a ^= c >> 12;
    b = (b - c) - a;
    b ^= a << 16;
    c = (c - a) - b;
    c ^= b >> 5;
    g_seed0 = (b - c) - a;
    g_seed0 ^= a >> 3;
    b = (b - a) - g_seed0;
    b ^= g_seed0 << 10;
    g_seed2 = (a - g_seed0) - b;
    g_seed2 ^= b >> 15;
    g_seed1 = b;

    mixed = (static_cast<uint64_t>(c ^ (b << 17)) << 32) | g_seed0;
    mixed = hash64shift(mixed);
    g_seed2 = static_cast<uint32_t>(mixed);
    g_seed3 = static_cast<uint32_t>(mixed >> 32);
    mixed = hash64shift((static_cast<uint64_t>(static_cast<uint32_t>(mixed)) << 32) | g_seed1);
    g_seed0 = static_cast<uint32_t>(mixed >> 32);
    g_seed1 = static_cast<uint32_t>(mixed);
}

uint32_t __N_16_crypto_iSeed_cpp_03964d54::AddToSeed(unsigned char *data, unsigned int length)
{
    unsigned int index;

    if (data == 0 || length == 0)
        return 0;
    for (index = 0; index < length; ++index)
    {
        g_seed0 ^= data[index];
        Stir();
    }
    return 1;
}
