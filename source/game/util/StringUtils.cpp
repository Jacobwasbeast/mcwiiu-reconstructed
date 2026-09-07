#include "game/util/StringUtils.hpp"

#include <cmath>
#include <limits>

int atoi_with_len__FPCciPi(const char* text, int base, int* length)
{
    const char* current = text;
    const bool negative = *text == '-';
    if (negative || *text == '+')
    {
        current = text + 1;
    }

    int value = 0;
    if (base == 0)
    {
        while (*current >= '0' && *current <= '9')
        {
            value = value * 10 + (*current - '0');
            ++current;
        }
    }
    else
    {
        while (true)
        {
            while (*current >= '0' && *current <= '9')
            {
                value = value * 16 + (*current - '0');
                ++current;
            }
            while (*current >= 'A' && *current <= 'F')
            {
                value = value * 16 + (*current - 'A' + 10);
                ++current;
            }
            if (*current < 'a' || *current > 'f')
            {
                break;
            }
            value = value * 16 + (*current - 'a' + 10);
            ++current;
        }
    }

    if (negative)
    {
        value = -value;
    }
    if (length != 0)
    {
        *length = static_cast<int>(current - text);
    }
    return value;
}

double atof_with_len__FPCcPi(const char* text, int* length)
{
    const char* current = text;
    const bool negative = *text == '-';
    if (negative || *text == '+')
    {
        ++current;
    }

    while (*current == '0')
    {
        ++current;
    }

    const char* integerBegin = current;
    while (*current >= '0' && *current <= '9')
    {
        ++current;
    }
    const char* integerEnd = current;

    const char* fractionBegin = current;
    const char* fractionEnd = current;
    if (*current == '.')
    {
        ++current;
        fractionBegin = current;
        while (*current == '0')
        {
            ++current;
        }
        fractionBegin = current;
        while (*current >= '0' && *current <= '9')
        {
            ++current;
        }
        fractionEnd = current;
    }

    int exponent = 0;
    const char* exponentEnd = current;
    if (*current == 'e' || *current == 'E')
    {
        ++current;
        const bool negativeExponent = *current == '-';
        if (negativeExponent || *current == '+')
        {
            ++current;
        }
        while (*current >= '0' && *current <= '9')
        {
            exponent = exponent * 10 + (*current - '0');
            ++current;
        }
        if (negativeExponent)
        {
            exponent = -exponent;
        }
        exponentEnd = current;
    }

    double value = 0.0;
    const char* digit = integerEnd;
    int place = 0;
    while (digit > integerBegin)
    {
        --digit;
        value += static_cast<double>(*digit - '0') * std::pow(10.0, static_cast<double>(place));
        ++place;
    }

    place = -1;
    for (digit = fractionBegin; digit < fractionEnd; ++digit)
    {
        value += static_cast<double>(*digit - '0') * std::pow(10.0, static_cast<double>(place));
        --place;
    }

    value *= std::pow(10.0, static_cast<double>(exponent));
    if (value > static_cast<double>(std::numeric_limits<float>::max()))
    {
        value = static_cast<double>(std::numeric_limits<float>::max());
    }
    if (negative)
    {
        value = -value;
    }
    if (length != 0)
    {
        *length = static_cast<int>((exponentEnd == current ? current : exponentEnd) - text);
    }
    return static_cast<double>(static_cast<float>(value));
}
