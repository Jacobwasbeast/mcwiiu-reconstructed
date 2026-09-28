#include "game/core/CAchievements.hpp"

#include <cstdio>
#include <cstring>
#include <string>

#include "platform/wiiu/CProfile.hpp"

int getSlotId(int iQuad);
PlayerUID getPlayerUID(int iQuad, bool includeOffline);

CAchievements::CAchievements(int iQuad)
    : mPlayerUID(0)
{
    mIsSystemPlayer = getSlotId(iQuad) != 0;
    mSlotId = (unsigned char)getSlotId(iQuad);
    mPlayerUID = getPlayerUID(iQuad, false);
    mDirty = false;
    std::memset(mAwarded, 0, sizeof(mAwarded));
    mState = 0;
    std::memset(mFilename, 0, sizeof(mFilename));
    mHasAchievementsFile = false;

    if (!mIsSystemPlayer)
    {
        PlayerUID playerUID = getPlayerUID(iQuad, false);
        const char* guest = (playerUID.flags & 0x20000000u) != 0 ? "True" : "False";
        gProfile->DebugPrintf("<CAchievements> Non-system player quad %i, guest=%s.\n",
                              iQuad, guest);
    }
    else
    {
        std::wstring uid = mPlayerUID.toString();
        gProfile->DebugPrintf("<CAchievements> Creating achievements for '%ls', iQuad=%i, slot=%i.\n",
                              uid.c_str(), iQuad, mSlotId);
        std::sprintf(mFilename, "ach_%ls", uid.c_str());
        StartRead();
    }
}

bool CAchievements::IsIdle() const
{
    return mState == 0;
}

bool CAchievements::StartRead()
{
    if (!IsIdle())
    {
        return false;
    }
    if (!mIsSystemPlayer)
    {
        return true;
    }
    mState = 1;
    gProfile->AddAwardManagerTask(this);
    return true;
}

void CAchievements::printAchievements() const
{
    std::wstring uid = mPlayerUID.toString();
    gProfile->DebugPrintf("<CAwardManager> Achievements for %ls.", uid.c_str());
    for (int first = 0; first < 128; first += 8)
    {
        gProfile->DebugPrintf("\n0x%03x| ", first);
        for (int bit = 0; bit < 8; ++bit)
        {
            gProfile->DebugPrintf("%s", (mAwarded[(first + bit) >> 3] & (1 << ((first + bit) & 7))) ? "1" : "0");
        }
    }
    gProfile->DebugPrintf("\n");
}

bool CAchievements::IsDirty() const
{
    return mDirty;
}

bool CAchievements::SetAwardedBit(int achievementId, bool awarded)
{
    int registrationId = gProfile->GetRegistrationId(achievementId);
    if (registrationId < 0)
    {
        return false;
    }

    unsigned char& value = mAwarded[registrationId >> 3];
    unsigned char mask = (unsigned char)(1 << (registrationId & 7));
    bool wasAwarded = (value & mask) != 0;
    if (awarded)
    {
        value |= mask;
    }
    else
    {
        value &= (unsigned char)~mask;
    }
    if (!mDirty && wasAwarded == awarded)
    {
        mDirty = false;
    }
    else
    {
        mDirty = true;
    }
    return wasAwarded;
}

bool CAchievements::GetAwardedBit(int achievementId) const
{
    int registrationId = gProfile->GetRegistrationId(achievementId);
    if (registrationId < 0)
    {
        return false;
    }
    return (mAwarded[registrationId >> 3] & (1 << (registrationId & 7))) != 0;
}

bool CAchievements::hasPendingAwards() const
{
    return !mPendingAwards.empty();
}

void CAchievements::addPendingAward(int achievementId)
{
    mPendingAwards.insert(achievementId);
}

void CAchievements::awardPending(std::set<int>& awardsNotRegistered)
{
    if (!hasPendingAwards())
    {
        return;
    }

    std::set<int> pendingAwards = mPendingAwards;
    mPendingAwards.clear();
    for (std::set<int>::const_iterator it = pendingAwards.begin(); it != pendingAwards.end(); ++it)
    {
        if (!SetAwardedBit(*it, true))
        {
            awardsNotRegistered.insert(*it);
        }
    }
}

unsigned int CAchievements::GetState() const
{
    return mState;
}

const char* CAchievements::GetFilename() const
{
    return mFilename;
}

void CAchievements::WriteTo(void* data, int size)
{
    if (data != NULL)
    {
        std::memcpy(data, mAwarded, 16);
        gProfile->DebugPrintf("<CAwardManager> Achievement file written.\n");
        printAchievements();
    }
    mState = 0;
    mDirty = false;
}

void CAchievements::ReadFrom(void* data, int size)
{
    if (data != NULL)
    {
        std::memcpy(mAwarded, data, 16);
        gProfile->DebugPrintf("<CAwardManager> Achievement file read.\n");
        printAchievements();
    }
    mState = 0;
    mDirty = false;
}

bool CAchievements::StartWrite()
{
    if (!IsIdle())
    {
        return false;
    }
    if (!mDirty)
    {
        return true;
    }
    if (!mIsSystemPlayer)
    {
        mDirty = false;
        return true;
    }
    mState = 2;
    gProfile->AddAwardManagerTask(this);
    return true;
}
