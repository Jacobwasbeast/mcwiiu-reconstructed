#include "game/util/StringUtils.hpp"

static const int POWER_TABLE_SIZE = 79;
static const int POWER_TABLE_UNIT_INDEX = 40;

static const double POWERS_OF_TEN[POWER_TABLE_SIZE] = {
    0.0, 1e-39, 1e-38, 1e-37, 1e-36, 1e-35,
    1e-34, 1e-33, 1e-32, 1e-31, 1e-30, 1e-29,
    1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23,
    1e-22, 1e-21, 1e-20, 1e-19, 1e-18, 1e-17,
    1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11,
    1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5,
    1e-4, 1e-3, 1e-2, 1e-1, 1e0, 1e1,
    1e2, 1e3, 1e4, 1e5, 1e6, 1e7,
    1e8, 1e9, 1e10, 1e11, 1e12, 1e13,
    1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
    1e20, 1e21, 1e22, 1e23, 1e24, 1e25,
    1e26, 1e27, 1e28, 1e29, 1e30, 1e31,
    1e32, 1e33, 1e34, 1e35, 1e36, 1e37,
    1e38
};

static const double DIGIT_VALUES[10] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};

static const double MAX_PARSED_VALUE = 3.40282e+38;

int atoi_with_len(const char* text, int base, int* length)
{
    const char* current = text;
    const bool negative = *text == '-';
    int character;
    if (negative || (character = *text) == '+')
    {
        current = text + 1;
        character = *current;
    }

    int value = 0;
    if (base != 0)
    {
        while (true)
        {
            while (true)
            {
                while (static_cast<unsigned int>(character - '0') < 10)
                {
                    ++current;
                    value = value * 16 + static_cast<unsigned int>(character - '0');
                    character = *current;
                }
                if (static_cast<unsigned int>(character - 'A') >= 6)
                {
                    break;
                }
                ++current;
                value = value * 16 + (character - 'A' + 10);
                character = *current;
            }
            if (static_cast<unsigned int>(character - 'a') >= 6)
            {
                break;
            }
            ++current;
            value = value * 16 + (character - 'a' + 10);
            character = *current;
        }
    }
    else
    {
        while (character >= '0' && character <= '9')
        {
            ++current;
            value = value * 10 + (character - '0');
            character = *current;
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

float atof_with_len(const char* text, int* length)
{
    const char* integerBegin = text;
    const bool negative = *text == '-';
    int character;
    if (negative || (character = *text) == '+')
    {
        ++integerBegin;
        character = *integerBegin;
    }
    while (character == '0')
    {
        ++integerBegin;
        character = *integerBegin;
    }

    const char* integerEnd = integerBegin;
    const char* lastIntegerDigit = integerBegin;
    if (static_cast<unsigned int>(character - '0') < 10)
    {
        while (true)
        {
            lastIntegerDigit = integerEnd;
            ++integerEnd;
            character = *integerEnd;
            while (character == '0')
            {
                ++integerEnd;
                character = *integerEnd;
            }
            if (static_cast<unsigned int>(character - '0') >= 10)
            {
                break;
            }
        }
    }

    const char* fractionBegin = integerEnd;
    const char* firstFractionDigit = integerEnd;
    const char* fractionEnd = integerEnd;
    const char* lastFractionDigit = integerEnd;
    if (character == '.')
    {
        fractionBegin = integerEnd + 1;
        character = *fractionBegin;
        firstFractionDigit = fractionBegin;
        while (character == '0')
        {
            ++firstFractionDigit;
            character = *firstFractionDigit;
        }
        fractionEnd = firstFractionDigit;
        lastFractionDigit = firstFractionDigit;
        if (static_cast<unsigned int>(character - '0') < 10)
        {
            while (true)
            {
                lastFractionDigit = fractionEnd;
                ++fractionEnd;
                character = *fractionEnd;
                while (character == '0')
                {
                    ++fractionEnd;
                    character = *fractionEnd;
                }
                if (static_cast<unsigned int>(character - '0') >= 10)
                {
                    break;
                }
            }
        }
    }

    int exponent = 0;
    const char* end = fractionEnd;
    if (character == 'e' || character == 'E')
    {
        end = fractionEnd + 1;
        character = *end;
        const bool negativeExponent = character == '-';
        if (character == '-' || character == '+')
        {
            ++end;
            character = *end;
        }
        while (static_cast<unsigned int>(character - '0') < 10)
        {
            exponent = exponent * 10 + character - '0';
            ++end;
            character = *end;
        }
        if (negativeExponent)
        {
            exponent = -exponent;
        }
    }

    double integerValue = 0.0;
    if (integerBegin != integerEnd)
    {
        integerValue = MAX_PARSED_VALUE;
        const int topIndex = static_cast<int>(integerEnd - integerBegin) + exponent + POWER_TABLE_UNIT_INDEX - 1;
        if (topIndex < POWER_TABLE_SIZE)
        {
            integerValue = 0.0;
            int index = topIndex + static_cast<int>(integerBegin - lastIntegerDigit);
            for (const char* digit = lastIntegerDigit; digit >= integerBegin; --digit)
            {
                if (index > 0)
                {
                    integerValue += POWERS_OF_TEN[index] * DIGIT_VALUES[*digit - '0'];
                }
                ++index;
            }
        }
    }

    double value = integerValue;
    if (firstFractionDigit != fractionEnd)
    {
        value = MAX_PARSED_VALUE;
        const int firstIndex = POWER_TABLE_UNIT_INDEX - 1 + static_cast<int>(fractionBegin - firstFractionDigit) + exponent;
        if (firstIndex < POWER_TABLE_SIZE)
        {
            double fractionValue = 0.0;
            int index = firstIndex + static_cast<int>(firstFractionDigit - lastFractionDigit);
            for (const char* digit = lastFractionDigit; digit >= firstFractionDigit; --digit)
            {
                if (index > 0)
                {
                    fractionValue += POWERS_OF_TEN[index] * DIGIT_VALUES[*digit - '0'];
                }
                ++index;
            }
            value = integerValue + fractionValue;
        }
    }

    if (negative)
    {
        value = -value;
    }
    if (length != 0)
    {
        *length = static_cast<int>(end - text);
    }
    return static_cast<float>(value);
}
