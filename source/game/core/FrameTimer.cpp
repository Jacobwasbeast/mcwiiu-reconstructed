#include "game/core/FrameTimer.hpp"

FrameTimer::FrameTimer()
{
    mLogStart = 0;
    mLogLength = 0;
    mLogEnd = 0;
}

int FrameTimer::wrapIndex(int index)
{
    return index % 240;
}

void FrameTimer::logFrameDuration(long long duration)
{
    mLoggedTimes[mLogEnd] = duration;
    ++mLogEnd;

    if (mLogEnd == 240)
    {
        mLogEnd = 0;
    }

    if (mLogLength < 240)
    {
        mLogStart = 0;
        ++mLogLength;
    }
    else
    {
        mLogStart = wrapIndex(mLogEnd + 1);
    }
}

int FrameTimer::getAverageDuration(int startIndex)
{
    long long total = 0;
    int count = startIndex;
    if (count > mLogLength)
        count = mLogLength;
    if (count <= 0)
        return 0;

    int index = mLogStart;
    for (int i = 0; i < count; ++i)
    {
        total += mLoggedTimes[index];
        index = wrapIndex(index + 1);
    }
    return static_cast<int>(total / count);
}

int FrameTimer::scaleSampleTo(long sample, int scale)
{
    return static_cast<int>((static_cast<double>(sample) / 16666666.0) * scale);
}

int FrameTimer::scaleAverageDurationTo(int startIndex, int scale)
{
    return scaleSampleTo(getAverageDuration(startIndex), scale);
}

int FrameTimer::getLogStart()
{
    return mLogStart;
}

int FrameTimer::getLogEnd()
{
    return mLogEnd;
}

long long* FrameTimer::getLog()
{
    return mLoggedTimes;
}
