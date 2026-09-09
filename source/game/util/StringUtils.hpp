#ifndef GAME_UTIL_STRINGUTILS_HPP
#define GAME_UTIL_STRINGUTILS_HPP

#include <sstream>
#include <string>

template <typename T>
std::wstring toString(T value)
{
    std::wostringstream stream;
    stream << value;
    return stream.str();
}

int atoi_with_len__FPCciPi(const char* text, int base, int* length);
double atof_with_len__FPCcPi(const char* text, int* length);

#endif
