#include "platform/wiiu/FFLiMiddleDB.hpp"
#include "platform/wiiu/FFLiMiiDataOfficial.hpp"
#include <string.h>

void FFLiMiiDataOfficial2CharInfo(FFLiCharInfo* charInfo, const FFLiMiiDataOfficial& data);
void FFLiCharInfo2MiiDataOfficial(FFLiMiiDataOfficial* data, const FFLiCharInfo& charInfo, bool includeCreator);

unsigned short FFLiMiddleDB::GetBufferSize(unsigned short count)
{
    return static_cast<unsigned short>(count * sizeof(FFLiMiiDataOfficial));
}

void FFLiMiddleDB::Init(FFLMiddleDBType type, void* buffer, unsigned short size)
{
    mType = type;
    mData = buffer;
    mStoredSize = size;
    ClearData();
}

void FFLiMiddleDB::SetHiddenParam(FFLGender gender)
{
    mParameters[0] = gender;
}

void FFLiMiddleDB::SetRandomParam(FFLGender gender, FFLAge age, FFLRace race)
{
    mParameters[0] = gender;
    mParameters[1] = age;
    mParameters[2] = race;
}

unsigned short FFLiMiddleDB::Size() const
{
    return mStoredSize;
}

FFLMiddleDBType FFLiMiddleDB::Type() const
{
    return mType;
}

void FFLiMiddleDB::ClearData()
{
    if (mData != 0)
        memset(mData, 0, GetBufferSize(mStoredSize));
}

void FFLiMiddleDB::SetNetParam(unsigned short index)
{
    mParameters[0] = static_cast<unsigned char>(index & 255);
    mParameters[1] = static_cast<unsigned char>(index >> 8);
}

const FFLiMiddleDBHiddenParam* FFLiMiddleDB::HiddenParam() const
{
    return (const FFLiMiddleDBHiddenParam*)mParameters;
}

const FFLiMiddleDBRandomParam* FFLiMiddleDB::RandomParam() const
{
    return (const FFLiMiddleDBRandomParam*)mParameters;
}

const FFLiMiddleDBNetParam* FFLiMiddleDB::NetParam() const
{
    return (const FFLiMiddleDBNetParam*)mParameters;
}

bool FFLiMiddleDB::IsFull() const
{
    return mData == 0 || mStoredSize == 0;
}

void FFLiMiddleDB::Add(const FFLiCharInfo& charInfo)
{
    if (mData != 0 && mStoredSize != 0)
        FFLiCharInfo2MiiDataOfficial(static_cast<FFLiMiiDataOfficial*>(mData), charInfo, true);
}

void FFLiMiddleDB::Add(const FFLiMiiDataHidden& miiData)
{
    if (mData != 0 && mStoredSize != 0)
        memcpy(mData, &miiData, sizeof(FFLiMiiDataOfficial));
}

FFLiMiiDataOfficial* FFLiMiddleDB::Get(unsigned short index)
{
    return static_cast<FFLiMiiDataOfficial*>(mData) + index;
}

const FFLiMiiDataOfficial* FFLiMiddleDB::Get(unsigned short index) const
{
    return static_cast<const FFLiMiiDataOfficial*>(mData) + index;
}

unsigned short FFLiMiddleDB::StoredSize() const
{
    return mStoredSize;
}

bool FFLiMiddleDB::GetCharInfo(FFLiCharInfo* charInfo, unsigned short index) const
{
    if (StoredSize() <= index)
        return true;
    FFLiMiiDataOfficial2CharInfo(charInfo, *Get(index));
    return false;
}

