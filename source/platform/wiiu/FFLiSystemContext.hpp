#ifndef PLATFORM_WIIU_FFLISYSTEMCONTEXT_HPP
#define PLATFORM_WIIU_FFLISYSTEMCONTEXT_HPP

#include "platform/wiiu/FFLiRandomContext.hpp"

struct FFLiCreateIDBase
{
    unsigned int words[2];
};

class FFLiSystemContext : public FFLiRandomContext
{
public:
    FFLiSystemContext();
    FFLiSystemContext(unsigned int seed);
    ~FFLiSystemContext();
    void Init(unsigned int seed);
    const FFLiCreateIDBase* UserID() const;
    const FFLiCreateIDBase* GetCreateIDBase() const;
    void SetCreateIDBase(const FFLiCreateIDBase* createIDBase);
    unsigned short Random(unsigned short maximum);
    unsigned long long TitleID() const;
    unsigned int AfterConstruct();

private:
    FFLiCreateIDBase mCreateIDBase;
    unsigned char mStatus14;
    unsigned char mStatus15;
    unsigned char mStatus16;
    unsigned char mStatus17;
    unsigned char mStatus18;
    unsigned char mStatus19;
    unsigned char mReserved1a[6];
    unsigned int mTitleIDLow;
    unsigned int mTitleIDHigh;
};

#endif
