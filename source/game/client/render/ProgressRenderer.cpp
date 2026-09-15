#include "game/client/render/ProgressRenderer.hpp"

#include <cwchar>

#include "game/core/CriticalSection.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/System.hpp"

static OSFastMutex sProgressRendererMutex;

ProgressRenderer::TextField::TextField()
    : active(false), id(-1), text()
{
}

ProgressRenderer::TextField::TextField(const TextField& other)
    : active(other.active), id(other.id), text(other.text)
{
}

ProgressRenderer::ProgressRenderer(Minecraft* minecraft)
    : mStatus(), mMinecraft(minecraft), mTitle(), mStartTime(0), mNoAbort(false)
{
    mStartTime = static_cast<unsigned long long>(System::processTimeInMilliSecs());
}

ProgressRenderer::~ProgressRenderer() = default;

void ProgressRenderer::internalProgressStart(const TextField& field)
{
    if (mMinecraft->mAppPaused || !mNoAbort)
    {
        EnterCriticalSection(&sProgressRendererMutex);
        mPercent = 0;
        if (!field.active)
        {
            mTitle.active = false;
            mTitle.id = field.id;
            mTitle.text = field.text;
        }
        else if (field.id != -2)
        {
            mTitle.active = true;
            mTitle.id = field.id;
            mTitle.text = field.text;
        }
        else
        {
            mTitle.active = true;
            mTitle.id = -2;
        }
        LeaveCriticalSection(&sProgressRendererMutex);
    }
}

void ProgressRenderer::progressStart(int id)
{
    mNoAbort = false;
    TextField field;
    field.active = true;
    field.id = id;
    field.text = L10N::GetString(id);
    internalProgressStart(field);
}

void ProgressRenderer::progressStartNoAbort(int id)
{
    mNoAbort = true;
    TextField field;
    field.active = true;
    field.id = id;
    field.text = L10N::GetString(id);
    internalProgressStart(field);
}

void ProgressRenderer::progressStartNoAbort(const std::wstring& text)
{
    mNoAbort = true;
    TextField field;
    field.active = false;
    field.id = -1;
    field.text = text;
    internalProgressStart(field);
}

void ProgressRenderer::progressStart(const std::wstring& text)
{
    mNoAbort = false;
    TextField field;
    field.active = false;
    field.id = -1;
    field.text = text;
    internalProgressStart(field);
}

void ProgressRenderer::progressStage(int id)
{
    if (mMinecraft->mAppPaused || !mNoAbort)
    {
        mStartTime = 0;
        mPercent = 0;
        EnterCriticalSection(&sProgressRendererMutex);
        mStatus.active = true;
        mStatus.id = id;
        mStatus.text = L10N::GetString(id);
        LeaveCriticalSection(&sProgressRendererMutex);
        mStartTime = 0;
        mPercent = 0;
    }
}

void ProgressRenderer::progressStage(const std::wstring& text)
{
    if (mMinecraft->mAppPaused || !mNoAbort)
    {
        mStartTime = 0;
        mPercent = 0;
        EnterCriticalSection(&sProgressRendererMutex);
        mStatus.id = -1;
        mStatus.active = false;
        mStatus.text = text;
        LeaveCriticalSection(&sProgressRendererMutex);
        mStartTime = 0;
        mPercent = 0;
    }
}

void ProgressRenderer::progressStagePercentage(int percentage)
{
    EnterCriticalSection(&sProgressRendererMutex);
    mPercent = percentage;
    LeaveCriticalSection(&sProgressRendererMutex);
}

int ProgressRenderer::getCurrentTitle()
{
    EnterCriticalSection(&sProgressRendererMutex);
    TextField field(mTitle);
    LeaveCriticalSection(&sProgressRendererMutex);
    return field.active ? field.id : -1;
}

std::wstring ProgressRenderer::getCurrentTitleString()
{
    EnterCriticalSection(&sProgressRendererMutex);
    std::wstring result(mTitle.text);
    LeaveCriticalSection(&sProgressRendererMutex);
    return result;
}

std::wstring ProgressRenderer::getCurrentStatusString()
{
    EnterCriticalSection(&sProgressRendererMutex);
    std::wstring result(mStatus.text);
    LeaveCriticalSection(&sProgressRendererMutex);
    return result;
}

int ProgressRenderer::getCurrentPercent()
{
    EnterCriticalSection(&sProgressRendererMutex);
    int result = mPercent;
    LeaveCriticalSection(&sProgressRendererMutex);
    return result;
}

int ProgressRenderer::getCurrentStatus()
{
    EnterCriticalSection(&sProgressRendererMutex);
    int result = mStatus.active ? mStatus.id : -1;
    LeaveCriticalSection(&sProgressRendererMutex);
    return result;
}
