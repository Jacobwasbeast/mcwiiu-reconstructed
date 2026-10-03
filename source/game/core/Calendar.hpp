#ifndef GAME_CORE_CALENDAR_HPP
#define GAME_CORE_CALENDAR_HPP

class Calendar
{
public:
    static int GetDayOfMonth();
    static int GetMonth();
};

extern "C" int IsLeapYear(int year);
extern "C" int GetYearDays(int year, int month);
extern "C" int GetLeapDays(int year);

#endif
