#include "game/core/StringConversion.hpp"

#include <cstdlib>
#include <cwchar>
#include <iomanip>
#include <sstream>

namespace
{
    void appendCodePoint(std::wstring& result, unsigned int codePoint)
    {
        if (sizeof(wchar_t) == 2 && codePoint > 0xffff)
        {
            codePoint -= 0x10000;
            result.push_back(static_cast<wchar_t>(0xd800 + (codePoint >> 10)));
            result.push_back(static_cast<wchar_t>(0xdc00 + (codePoint & 0x3ff)));
        }
        else
        {
            result.push_back(static_cast<wchar_t>(codePoint));
        }
    }
}

std::wstring char16towstring(wchar_t* text)
{
    return text == NULL ? std::wstring() : std::wstring(text);
}

void charTowchar_tArray(const char* text, wchar_t* destination, unsigned int capacity)
{
    if (text == NULL || destination == NULL || capacity == 0)
        return;

    unsigned int index = 0;
    while (index < capacity)
    {
        destination[index] = static_cast<signed char>(text[index]);
        if (text[index] == '\0')
            return;
        ++index;
    }
}

bool compareStrings(const std::wstring& first, const std::wstring& second)
{
    return std::wcscmp(first.c_str(), second.c_str()) < 0;
}

wchar_t* stringtowchararray(const std::string& text)
{
    static wchar_t buffer[256];

    const std::string::size_type length = text.size();
    for (std::string::size_type index = 0; index < length; ++index)
        buffer[index] = static_cast<signed char>(text[index]);
    buffer[length] = 0;
    return buffer;
}

std::wstring convStringToWstring(wchar_t* text)
{
    return text == NULL ? std::wstring() : std::wstring(text);
}

std::wstring convStringToWstring(const std::string& text)
{
    return convUtf8ToWstring(text.c_str());
}

std::wstring convUtf8ToWstring(const char* text)
{
    std::wstring result;
    if (text == NULL)
        return result;

    const unsigned char* current = reinterpret_cast<const unsigned char*>(text);
    while (*current != 0)
    {
        unsigned int codePoint;
        unsigned int length;
        if (*current < 0x80)
        {
            codePoint = *current;
            length = 1;
        }
        else if ((*current & 0xe0) == 0xc0 && (current[1] & 0xc0) == 0x80)
        {
            codePoint = (*current & 0x1f) << 6 | (current[1] & 0x3f);
            length = 2;
        }
        else if ((*current & 0xf0) == 0xe0 && (current[1] & 0xc0) == 0x80 &&
                 (current[2] & 0xc0) == 0x80)
        {
            codePoint = (*current & 0x0f) << 12 | (current[1] & 0x3f) << 6 |
                        (current[2] & 0x3f);
            length = 3;
        }
        else if ((*current & 0xf8) == 0xf0 && (current[1] & 0xc0) == 0x80 &&
                 (current[2] & 0xc0) == 0x80 && (current[3] & 0xc0) == 0x80)
        {
            codePoint = (*current & 0x07) << 18 | (current[1] & 0x3f) << 12 |
                        (current[2] & 0x3f) << 6 | (current[3] & 0x3f);
            length = 4;
        }
        else
        {
            codePoint = 0xfffd;
            length = 1;
        }
        appendCodePoint(result, codePoint);
        current += length;
    }
    return result;
}

std::string convWstringToString(const std::wstring& text)
{
    std::string result;
    result.reserve(text.size());
    for (std::wstring::const_iterator it = text.begin(); it != text.end(); ++it)
    {
        unsigned int codePoint = static_cast<unsigned int>(*it);
        if (codePoint < 0x80)
            result.push_back(static_cast<char>(codePoint));
        else if (codePoint < 0x800)
        {
            result.push_back(static_cast<char>(0xc0 | (codePoint >> 6)));
            result.push_back(static_cast<char>(0x80 | (codePoint & 0x3f)));
        }
        else if (codePoint < 0x10000)
        {
            result.push_back(static_cast<char>(0xe0 | (codePoint >> 12)));
            result.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (codePoint & 0x3f)));
        }
        else
        {
            result.push_back(static_cast<char>(0xf0 | (codePoint >> 18)));
            result.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (codePoint & 0x3f)));
        }
    }
    return result;
}

template <typename T>
std::wstring _toString(T value)
{
    std::wostringstream stream;
    stream << value;
    return stream.str();
}

std::wstring _toString(int value)
{
    return _toString<int>(value);
}

std::wstring _toString(float value)
{
    return _toString<float>(value);
}

template <typename T>
T _fromString(const std::wstring& value)
{
    std::wistringstream stream(value);
    T result = T();
    stream >> result;
    return result;
}

template <typename T>
std::wstring _toHexString(T value)
{
    std::wostringstream stream;
    stream << std::hex << value;
    return stream.str();
}

template <typename T>
T _fromHEXString(const std::wstring& value)
{
    std::wistringstream stream(value);
    T result = T();
    stream >> std::hex >> result;
    return result;
}

template std::wstring _toString<bool>(bool);
template std::wstring _toString<short>(short);
template std::wstring _toString<unsigned short>(unsigned short);
template std::wstring _toString<unsigned char>(unsigned char);
template std::wstring _toString<int>(int);
template std::wstring _toString<unsigned int>(unsigned int);
template std::wstring _toString<float>(float);
template std::wstring _toString<double>(double);
template std::wstring _toString<long long>(long long);
template std::wstring _toString<unsigned long long>(unsigned long long);
template std::wstring _toString<const Direction*>(const Direction*);
template std::wstring _toString<Block*>(Block*);
template std::wstring _toString<const BlockState*>(const BlockState*);

template bool _fromString<bool>(const std::wstring&);
template unsigned short _fromString<unsigned short>(const std::wstring&);
template int _fromString<int>(const std::wstring&);
template unsigned int _fromString<unsigned int>(const std::wstring&);
template float _fromString<float>(const std::wstring&);
template double _fromString<double>(const std::wstring&);
template long long _fromString<long long>(const std::wstring&);

template std::wstring _toHexString<long long>(long long);
template long long _fromHEXString<long long>(const std::wstring&);
template int _fromHEXString<int>(const std::wstring&);
