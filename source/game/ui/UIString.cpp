#include "game/ui/UIString.hpp"

#include "game/core/IdsStringBuilder.hpp"
#include "game/core/StringConversion.hpp"

extern unsigned int XGetLanguage();
extern int XGetLocale();

static unsigned int gCurrentLanguage = static_cast<unsigned int>(-1);
static unsigned int gCurrentLocale = static_cast<unsigned int>(-1);

UIString::UIStringCore::UIStringCore(FJStringBuilder builder)
    : mLanguage(static_cast<unsigned int>(-1)),
      mLocale(static_cast<unsigned int>(-1)),
      mUpdatedLanguage(static_cast<unsigned int>(-1)),
      mUpdatedLocale(static_cast<unsigned int>(-1)),
      mIsConstant(false),
      mIsCharString(false)
{
    mBuilder = builder;
    mString = L"";
    update(true);
}

UIString::UIStringCore::UIStringCore(const std::wstring& value)
    : mLanguage(static_cast<unsigned int>(-1)),
      mLocale(static_cast<unsigned int>(-1)),
      mUpdatedLanguage(static_cast<unsigned int>(-1)),
      mUpdatedLocale(static_cast<unsigned int>(-1)),
      mIsConstant(true),
      mIsCharString(false)
{
    mString = value;
}

UIString::UIStringCore::UIStringCore(const std::string& value)
    : mLanguage(static_cast<unsigned int>(-1)),
      mLocale(static_cast<unsigned int>(-1)),
      mUpdatedLanguage(static_cast<unsigned int>(-1)),
      mUpdatedLocale(static_cast<unsigned int>(-1)),
      mIsConstant(true),
      mIsCharString(true)
{
    mCharString = value;
}

unsigned int UIString::UIStringCore::hasNewString()
{
    if (mIsConstant)
        return 0;
    if (mLanguage == gCurrentLanguage && mLocale == gCurrentLocale)
        return 0;
    return 1;
}

unsigned int UIString::UIStringCore::update(bool force)
{
    if (mIsConstant)
        return 0;
    if (!force && hasNewString() == 0)
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
    if (mLanguage == gCurrentLanguage && mUpdatedLanguage == mLanguage &&
        mLocale == gCurrentLocale && mUpdatedLocale == mLocale)
        return 0;
    return 1;
}

void UIString::UIStringCore::setUpdated()
{
    mUpdatedLanguage = mLanguage;
    mUpdatedLocale = mLocale;
}

std::wstring& UIString::UIStringCore::getString()
{
    if (hasNewString() != 0)
        update(true);
    return mString;
}

std::string& UIString::UIStringCore::getCharString()
{
    if (hasNewString() != 0)
        update(true);
    return mCharString;
}

unsigned char UIString::UIStringCore::isCharString()
{
    return mIsCharString;
}

UIString::UIString()
{
    mCore.reset();
}

UIString::UIString(int id)
{
    mCore.reset(new UIStringCore(FJStringBuilder(new IdsStringBuilder(id))));
}

UIString::UIString(FJStringBuilder builder)
{
    mCore.reset(new UIStringCore(builder));
}

UIString::UIString(const std::string& value)
{
    mCore.reset(new UIStringCore(convStringToWstring(value)));
}

UIString::UIString(const char* value)
{
    mCore.reset(new UIStringCore(std::string(value)));
}

UIString::UIString(const std::wstring& value)
{
    mCore.reset(new UIStringCore(value));
}

UIString::UIString(const wchar_t* value)
{
    mCore.reset(new UIStringCore(std::wstring(value)));
}

UIString::~UIString()
{
    mCore.reset();
}

unsigned int UIString::setCurrentLanguage()
{
    unsigned int language = XGetLanguage();
    unsigned int localeId = static_cast<unsigned int>(XGetLocale());
    if (language == gCurrentLanguage && localeId == gCurrentLocale)
        return 0;
    gCurrentLocale = localeId;
    gCurrentLanguage = language;
    return 1;
}

unsigned int UIString::getCurrentLanguage()
{
    return gCurrentLanguage;
}

bool UIString::empty()
{
    return !mCore;
}

bool UIString::compare(const UIString& value)
{
    return mCore != value.mCore;
}

unsigned int UIString::needsUpdating()
{
    if (!mCore)
        return 0;
    return mCore->needsUpdating();
}

void UIString::setUpdated()
{
    if (!mCore)
        return;
    mCore->setUpdated();
}

std::wstring& UIString::getString()
{
    static std::wstring emptyString(L"");
    if (mCore)
        return mCore->getString();
    return emptyString;
}

std::string& UIString::getCharString()
{
    static std::string emptyString("");
    if (mCore)
        return mCore->getCharString();
    return emptyString;
}

unsigned char UIString::isCharString()
{
    if (!mCore)
        return 0;
    return mCore->isCharString();
}

const wchar_t* UIString::c_str()
{
    return getString().c_str();
}
