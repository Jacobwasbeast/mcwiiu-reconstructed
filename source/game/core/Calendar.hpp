#ifndef GAME_CORE_CALENDAR_HPP
#define GAME_CORE_CALENDAR_HPP

class Calendar
{
public:
    static int GetDayOfMonth();
    static int GetMonth();
};

int GetYearDays(int year, int month);
int GetLeapDays(int year);

#endif
