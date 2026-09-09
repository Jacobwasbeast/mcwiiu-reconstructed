#include "game/core/Calendar.hpp"

#include <time.h>

int IsLeapYear(int year)
{
    return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 1 : 0;
}

int GetYearDays(int year, int month)
{
    static const int days[2][4] =
    {
        { 0, 31, 59, 90 },
        { 0, 31, 60, 91 }
    };

    return days[IsLeapYear(year) != 0][month];
}

int GetLeapDays(int year)
{
    return (year + 3) / 4 - (year - 1) / 100 + (year - 1) / 400;
}

int Calendar::GetDayOfMonth()
{
    time_t now = time(NULL);
    struct tm* local = localtime(&now);
    return local->tm_mday;
}

int Calendar::GetMonth()
{
    time_t now = time(NULL);
    struct tm* local = localtime(&now);
    return local->tm_mon + 1;
}
