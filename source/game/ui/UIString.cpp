#include "game/ui/UIString.hpp"
#include "game/core/IdsStringBuilder.hpp"

extern int XGetLanguage();
extern int XGetLocale();

static unsigned int gCurrentLanguage = static_cast<unsigned int>(-1);
static unsigned int gCurrentLocale = static_cast<unsigned int>(-1);

UIString::UIStringCore::UIStringCore(FJStringBuilder builder)
    : mLanguage(static_cast<unsigned int>(-1)), mLocale(static_cast<unsigned int>(-1)),
      mUpdatedLanguage(static_cast<unsigned int>(-1)), mUpdatedLocale(static_cast<unsigned int>(-1)),
      mString(L"Button1"), mCharString(), mIsCharString(false), mIsConstant(false), mBuilder(builder)
{
    update(true);
}

UIString::UIStringCore::UIStringCore(const std::wstring& value)
    : mLanguage(static_cast<unsigned int>(-1)), mLocale(static_cast<unsigned int>(-1)),
      mUpdatedLanguage(static_cast<unsigned int>(-1)), mUpdatedLocale(static_cast<unsigned int>(-1)),
      mString(value), mCharString(), mIsCharString(false), mIsConstant(true), mBuilder()
{
}

UIString::UIStringCore::UIStringCore(const std::string& value)
    : mLanguage(static_cast<unsigned int>(-1)), mLocale(static_cast<unsigned int>(-1)),
      mUpdatedLanguage(static_cast<unsigned int>(-1)), mUpdatedLocale(static_cast<unsigned int>(-1)),
      mString(), mCharString(value), mIsCharString(true), mIsConstant(true), mBuilder()
{
}

unsigned int UIString::UIStringCore::hasNewString()
{
    if (mIsConstant)
        return 0;
    return (mLanguage != gCurrentLanguage || mLocale != gCurrentLocale) ? 1 : 0;
}

unsigned int UIString::UIStringCore::update(bool force)
{
    if (mIsConstant || (!force && !hasNewString()))
        return 0;
    mString = mBuilder();
    mLanguage = gCurrentLanguage;
    mLocale = gCurrentLocale;
    return 1;
}

unsigned int UIString::UIStringCore::needsUpdating()
{
    if (mIsConstant)
        return 0;
    return (mLanguage != gCurrentLanguage || mUpdatedLanguage != mLanguage ||
            mLocale != gCurrentLocale || mUpdatedLocale != mLocale) ? 1 : 0;
}

void UIString::UIStringCore::setUpdated()
{
    mUpdatedLanguage = mLanguage;
    mUpdatedLocale = mLocale;
}

int UIString::UIStringCore::getString()
{
    if (hasNewString())
        update(true);
    return (int)(long)&mString;
}

int UIString::UIStringCore::getCharString()
{
    if (hasNewString())
        update(true);
    return (int)(long)&mCharString;
}

unsigned char UIString::UIStringCore::isCharString()
{
    return mIsCharString ? 1 : 0;
}

unsigned int UIString::UIStringCore::length()
{
    return mIsCharString ? static_cast<unsigned int>(mCharString.size()) : static_cast<unsigned int>(mString.size());
}

UIString::UIString() : mCore(0), mReferences(0)
{
}

UIString::UIString(int id) : mCore(0), mReferences(0)
{
    IdsStringBuilder ids(id);
    FJStringBuilder builder;
    mCore = new UIStringCore(builder);
}

UIString::UIString(FJStringBuilder builder) : mCore(new UIStringCore(builder)), mReferences(0)
{
}

UIString::UIString(const std::string& value) : mCore(new UIStringCore(value)), mReferences(0)
{
}

UIString::UIString(const char* value)
    : mCore(new UIStringCore(std::string(value ? value : ""))), mReferences(0)
{
}

UIString::UIString(const std::wstring& value) : mCore(new UIStringCore(value)), mReferences(0)
{
}

UIString::UIString(const wchar_t* value)
    : mCore(new UIStringCore(std::wstring(value ? value : L""))), mReferences(0)
{
}

UIString::~UIString()
{
    delete mCore;
}

unsigned int UIString::setCurrentLanguage()
{
    const unsigned int language = static_cast<unsigned int>(XGetLanguage());
    const unsigned int locale = static_cast<unsigned int>(XGetLocale());
    if (language == gCurrentLanguage && locale == gCurrentLocale)
        return 0;
    gCurrentLanguage = language;
    gCurrentLocale = locale;
    return 1;
}

unsigned int UIString::getCurrentLanguage()
{
    return gCurrentLanguage;
}

bool UIString::empty()
{
    return mCore == 0;
}

bool UIString::compare(const UIString& value)
{
    return mCore != value.mCore;
}

unsigned int UIString::needsUpdating()
{
    return mCore ? mCore->needsUpdating() : 0;
}

void UIString::setUpdated()
{
    if (mCore)
        mCore->setUpdated();
}

unsigned int UIString::getString()
{
    return mCore ? static_cast<unsigned int>(mCore->getString()) : 0;
}

unsigned int UIString::getCharString()
{
    return mCore ? static_cast<unsigned int>(mCore->getCharString()) : 0;
}

unsigned char UIString::isCharString()
{
    return mCore ? mCore->isCharString() : 0;
}

int UIString::c_str()
{
    return static_cast<int>(getString());
}

unsigned int UIString::length()
{
    return mCore ? mCore->length() : 0;
}
