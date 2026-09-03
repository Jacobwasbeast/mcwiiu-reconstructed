#include "platform/wiiu/FFLiOrderData.hpp"

void FFLiOrderData::Init()
{
    mNextIndex = 0xffff;
    mPrevIndex = 0xffff;
}

void FFLiOrderData::Clear()
{
    Init();
}

bool FFLiOrderData::IsLinkedNext() const
{
    return mNextIndex != 0xffff;
}

bool FFLiOrderData::IsLinkedPrev() const
{
    return mPrevIndex != 0xffff;
}

bool FFLiOrderData::IsUnlinked() const
{
    return !IsLinkedNext() && !IsLinkedPrev();
}

unsigned short FFLiOrderData::NextIndex() const
{
    return mNextIndex;
}

unsigned short FFLiOrderData::PrevIndex() const
{
    return mPrevIndex;
}

void FFLiOrderData::SetNextIndex(unsigned short index)
{
    mNextIndex = index;
}

void FFLiOrderData::SetPrevIndex(unsigned short index)
{
    mPrevIndex = index;
}

void FFLiOrderData::SwapEndian()
{
    mNextIndex = (unsigned short)((mNextIndex >> 8) | (mNextIndex << 8));
    mPrevIndex = (unsigned short)((mPrevIndex >> 8) | (mPrevIndex << 8));
}
