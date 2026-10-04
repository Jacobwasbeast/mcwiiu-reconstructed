#include "game/core/RoundInfo.hpp"
#include "game/util/Random.hpp"
#include "game/core/StringConversion.hpp"
#include "game/world/LevelGenerationOptions.hpp"

std::wstring CreateRoundGuid()
{
    Random random;
    return _toString<int>(random.nextInt());
}

RoundInfo::RoundInfo()
    : mRoundId(L"")
    , mSubRoundId(L"")
{
    mCurrentSubRoundNumber = -1;
    mThemeId = -1;
    mThemeSubId = -1;
}

void RoundInfo::reset()
{
    RoundInfo info;
    mRoundId = info.mRoundId;
    mSubRoundId = info.mSubRoundId;
    mCurrentSubRoundNumber = info.mCurrentSubRoundNumber;
    mThemeId = info.mThemeId;
    mThemeSubId = info.mThemeSubId;
}

void RoundInfo::startNewRound()
{
    mRoundId = CreateRoundGuid();
    mSubRoundId = CreateRoundGuid();
    mCurrentSubRoundNumber = 0;
}

void RoundInfo::startNewSubRound()
{
    mSubRoundId = CreateRoundGuid();
    ++mCurrentSubRoundNumber;
}

std::wstring RoundInfo::getRoundId() const
{
    return mRoundId;
}

std::wstring RoundInfo::getSubRoundId() const
{
    return mSubRoundId;
}

unsigned int RoundInfo::getCurrentSubRoundNumber() const
{
    return mCurrentSubRoundNumber;
}

unsigned int RoundInfo::getThemeId() const
{
    return mThemeId;
}

unsigned int RoundInfo::getThemeSubId() const
{
    return mThemeSubId;
}

unsigned int RoundInfo::getThemeUniqueId() const
{
    if (mThemeId >= 0 && mThemeSubId >= 0)
    {
        return LevelGenerationOptions::CreateUniqueId((int)mThemeId, (int)mThemeSubId);
    }

    return 0xffffffffU;
}

void RoundInfo::setTheme(int themeId, int themeSubId)
{
    mThemeId = themeId;
    mThemeSubId = themeSubId;
}

void RoundInfo::setUniqueThemeId(int uniqueThemeId)
{
    if (uniqueThemeId >= 0)
    {
        setTheme(LevelGenerationOptions::GetThemeFromUniqueId(uniqueThemeId),
                 LevelGenerationOptions::GetMapSizeFromUniqueId(uniqueThemeId));
    }
    else
    {
        mThemeId = 0xffffffffU;
        mThemeSubId = 0xffffffffU;
    }
}

void RoundInfo::setRoundAndSubRound(const std::wstring& roundId, const std::wstring& subRoundId)
{
    mRoundId = roundId;
    mSubRoundId = subRoundId;
}
