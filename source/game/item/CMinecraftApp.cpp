#include <string>

static char s_wideStringBuffer[1024];

char* wstringtochararray(const std::wstring& value)
{
    for (std::wstring::size_type index = 0; index < value.size(); ++index)
        s_wideStringBuffer[index] = static_cast<char>(value[index]);
    s_wideStringBuffer[value.size()] = 0;
    return s_wideStringBuffer;
}
