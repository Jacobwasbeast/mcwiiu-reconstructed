#include "platform/wiiu/FFLiSystemContext.hpp"

#include <cstring>

extern unsigned int FFLiGetSecondsFrom_2000_01_01__Fv();
extern unsigned long long FFLiGetMiiStudioTitleID__Fv();
extern int FFLiSystemContextGetCreateID(FFLiCreateIDBase*, unsigned int, unsigned char);
extern int FFLiSystemContextIsReady();

FFLiSystemContext::FFLiSystemContext()
{
    Init(FFLiGetSecondsFrom_2000_01_01__Fv());
}

FFLiSystemContext::FFLiSystemContext(unsigned int seed)
{
    Init(seed);
}

FFLiSystemContext::~FFLiSystemContext()
{
}

void FFLiSystemContext::Init(unsigned int seed)
{
    std::memset(&mCreateIDBase, 0, sizeof(mCreateIDBase));
    FFLiRandomContext::Init(seed);
    mStatus19 = 0xff;
    mStatus15 = 0xff;
    mStatus18 = 0xff;
    mStatus17 = 0xff;
    mStatus16 = 0xff;
    mStatus14 = 0xff;
    mTitleIDHigh = 0;
    mTitleIDLow = 0;
}

const FFLiCreateIDBase* FFLiSystemContext::UserID() const
{
    return &mCreateIDBase;
}

const FFLiCreateIDBase* FFLiSystemContext::GetCreateIDBase() const
{
    return &mCreateIDBase;
}

void FFLiSystemContext::SetCreateIDBase(const FFLiCreateIDBase* createIDBase)
{
    if (createIDBase != 0)
    {
        std::memcpy(&mCreateIDBase, createIDBase, sizeof(mCreateIDBase));
    }
}

unsigned short FFLiSystemContext::Random(unsigned short maximum)
{
    return FFLiRandomContext::Random(maximum);
}

unsigned long long FFLiSystemContext::TitleID() const
{
    return (static_cast<unsigned long long>(mTitleIDHigh) << 32) | mTitleIDLow;
}

unsigned int FFLiSystemContext::AfterConstruct()
{
    if (FFLiSystemContextIsReady() != 0)
    {
        return 0;
    }
    if (FFLiSystemContextGetCreateID(&mCreateIDBase, 0x4a0, 0xff) != 0)
    {
        return 0;
    }
    if (FFLiSystemContextIsReady() != 0)
    {
        return 0;
    }
    const unsigned long long titleID = FFLiGetMiiStudioTitleID__Fv();
    mTitleIDLow = static_cast<unsigned int>(titleID);
    mTitleIDHigh = static_cast<unsigned int>(titleID >> 32);
    return 1;
}
