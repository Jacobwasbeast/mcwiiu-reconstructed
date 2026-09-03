#include "platform/wiiu/FFLiRandomContext.hpp"

void FFLiRandomContext::Init(unsigned int seed)
{
    mSeed = seed;
    mMultiplier = 0x5d588b65;
    mIncrement = 0x269ec3;
}

unsigned short FFLiRandomContext::Random(unsigned short maximum)
{
    mSeed = mSeed * mMultiplier + mIncrement;
    if (maximum == 0)
    {
        return 0;
    }
    return static_cast<unsigned short>((mSeed >> 16) % maximum);
}
