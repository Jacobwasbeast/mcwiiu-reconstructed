#ifndef PLATFORM_WIIU_FFLIRANDOMCONTEXT_HPP
#define PLATFORM_WIIU_FFLIRANDOMCONTEXT_HPP

class FFLiRandomContext
{
public:
    void Init(unsigned int seed);
    unsigned short Random(unsigned short maximum);

private:
    unsigned int mSeed;
    unsigned int mMultiplier;
    unsigned int mIncrement;
};

#endif
