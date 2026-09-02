#include "game/core/Language.hpp"

static Language* sLanguage = new Language();

Language* Language::getInstance()
{
    return sLanguage;
}

std::wstring Language::getElement(const std::wstring& key, ...)
{
    return L"";
}
