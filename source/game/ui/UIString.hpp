#ifndef GAME_UI_UISTRING_HPP
#define GAME_UI_UISTRING_HPP

#include <string>
#include "game/core/FJStringBuilder.hpp"

class UIString
{
public:
    class UIStringCore
    {
    public:
        explicit UIStringCore(FJStringBuilder builder);
        explicit UIStringCore(const std::wstring& value);
        explicit UIStringCore(const std::string& value);
        unsigned int hasNewString();
        unsigned int update(bool force);
        unsigned int needsUpdating();
        void setUpdated();
        std::wstring& getString();
        int getCharString();
        unsigned char isCharString();
        unsigned int length();
    private:
        unsigned int mLanguage;
        unsigned int mLocale;
        unsigned int mUpdatedLanguage;
        unsigned int mUpdatedLocale;
        std::wstring mString;
        std::string mCharString;
        bool mIsCharString;
        bool mIsConstant;
        FJStringBuilder mBuilder;
    };
    UIString();
    explicit UIString(int id);
    explicit UIString(FJStringBuilder builder);
    explicit UIString(const std::string& value);
    explicit UIString(const char* value);
    explicit UIString(const std::wstring& value);
    explicit UIString(const wchar_t* value);
    ~UIString();
    static unsigned int setCurrentLanguage();
    static unsigned int getCurrentLanguage();
    bool empty();
    bool compare(const UIString& value);
    unsigned int needsUpdating();
    void setUpdated();
    std::wstring& getString();
    unsigned int getCharString();
    unsigned char isCharString();
    const wchar_t* c_str();
    unsigned int length();
private:
    UIStringCore* mCore;
    unsigned int mReferences;
};

#endif
