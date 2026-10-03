#include "game/core/Calendar.hpp"

#include <time.h>

extern "C" int IsLeapYear(int year)
{
    if ((year & 3) == 0 && year != year / 100 * 100)
        return 1;
    if (year == year / 400 * 400)
        return 1;
    return 0;
}

static int s_normalYearDays[4] = { 0, 31, 59, 90 };
static int s_leapYearDays[4] = { 0, 31, 60, 91 };

extern "C" int GetYearDays(int year, int month)
{
    int* days = s_normalYearDays;
    if (IsLeapYear(year) != 0)
        days = s_leapYearDays;
    return days[month];
}

extern "C" int GetLeapDays(int year)
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
