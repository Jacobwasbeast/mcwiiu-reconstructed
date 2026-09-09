#include "game/core/StringUtils.hpp"

#include <vector>
#include <sstream>
#include <algorithm>

std::vector<std::wstring> stringSplit(const std::wstring& text, wchar_t separator)
{
    std::vector<std::wstring> result;
    std::wistringstream stream(text);
    std::wstring field;
    while (std::getline(stream, field, separator))
    {
        result.push_back(field);
    }
    return result;
}

bool BothAreSpaces(wchar_t first, wchar_t second)
{
    return first == second && first == L' ';
}

std::wstring toLower(const std::wstring& text)
{
    std::wstring result(text);
    for (std::wstring::iterator it = result.begin(); it != result.end(); ++it)
    {
        if (*it >= L'A' && *it <= L'Z')
        {
            *it = static_cast<wchar_t>(*it + (L'a' - L'A'));
        }
    }
    return result;
}

std::wstring trimString(const std::wstring& text)
{
    const std::wstring whitespace(L" \t\n\r");
    const std::wstring::size_type first = text.find_first_not_of(whitespace);
    if (first == std::wstring::npos)
    {
        return std::wstring();
    }

    const std::wstring::size_type last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

void stripWhitespaceForHtml(std::wstring& text, bool collapse)
{
    if (collapse)
    {
        text.erase(std::remove(text.begin(), text.end(), L'\n'), text.end());
        text.erase(std::remove(text.begin(), text.end(), L'\r'), text.end());
    }
    text.erase(std::remove(text.begin(), text.end(), L'\t'), text.end());
    text.erase(std::unique(text.begin(), text.end(), BothAreSpaces), text.end());
    text = trimString(text);
}

void replaceAll(std::wstring& result, const std::wstring& source, const std::wstring& search, const std::wstring& replacement)
{
    result = source;
    std::wstring::size_type position = 0;
    while ((position = result.find(search, position)) != std::wstring::npos)
    {
        result.replace(position, search.length(), replacement);
        ++position;
    }
}

std::wstring replaceAll(const std::wstring& text, const std::wstring& search, const std::wstring& replacement)
{
    std::wstring result(text);
    std::wstring::size_type position = 0;
    while ((position = result.find(search, position)) != std::wstring::npos)
    {
        result.replace(position, search.length(), replacement);
        ++position;
    }
    return result;
}

void makeReplacements(const boost::unordered_map<std::wstring, std::wstring>& replacements, std::wstring& text)
{
    boost::unordered_map<std::wstring, std::wstring>::const_iterator it = replacements.begin();
    while (it != replacements.end())
    {
        text = replaceAll(text, it->first, it->second);
        ++it;
    }
}

void toUtf8(const std::wstring& text, char** output, unsigned int& length)
{
}
