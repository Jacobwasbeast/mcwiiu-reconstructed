#include "nn/nex/DateTime.hpp"

#include <coreinit/time.h>

namespace nn
{
namespace nex
{
namespace
{

const DateTime s_never;

}

DateTime::DateTime()
    : m_flags(0)
    , m_packed(0)
{
}

DateTime::DateTime(const DateTime& other)
    : m_flags(other.m_flags)
    , m_packed(other.m_packed)
{
}

DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day,
    uint8_t hour, uint8_t minute, uint8_t second)
{
    m_flags = (static_cast<uint32_t>(year) >> 6) & 0xff;
    m_packed = (static_cast<uint32_t>(second) & 0x3f)
        | ((static_cast<uint32_t>(minute) & 0x3f) << 6)
        | ((static_cast<uint32_t>(hour) & 0x1f) << 12)
        | ((static_cast<uint32_t>(day) & 0x1f) << 17)
        | ((static_cast<uint32_t>(month) & 0xf) << 22)
        | (static_cast<uint32_t>(year) << 26);

    if (year < 1900)
        m_flags |= 0x100;

    if (year > 9999
        || static_cast<uint32_t>(month) - 1 >= 12
        || static_cast<uint32_t>(day) - 1 >= 31)
    {
        m_flags = (m_flags & 0xfffffcffu) | 0x100;
        *this = s_never;
    }
}

DateTime& DateTime::operator=(const DateTime& other)
{
    m_flags = other.m_flags;
    m_packed = other.m_packed;
    return *this;
}

DateTime::operator unsigned long long()
{
    return (static_cast<uint64_t>(m_flags) << 32) | m_packed;
}

DateTime::operator unsigned long long() const
{
    return (static_cast<uint64_t>(m_flags) << 32) | m_packed;
}

uint64_t DateTime::operator-(const DateTime& other) const
{
    return (ToEpochTime() - other.ToEpochTime()) * 1000;
}

uint32_t DateTime::GetYear() const
{
    return ((m_flags & 0xff) << 6) | (m_packed >> 26);
}

uint32_t DateTime::GetMonth() const
{
    return (m_packed >> 22) & 0xf;
}

uint32_t DateTime::GetDay() const
{
    return (m_packed >> 17) & 0x1f;
}

uint32_t DateTime::GetHour() const
{
    return (m_packed >> 12) & 0x1f;
}

uint32_t DateTime::GetMinute() const
{
    return (m_packed >> 6) & 0x3f;
}

uint8_t DateTime::GetSecond() const
{
    return static_cast<uint8_t>(m_packed & 0x3f);
}

uint32_t DateTime::IsLeapYear(int32_t year) const
{
    if (year != (year / 400) * 400)
    {
        if (year == (year / 100) * 100 || (year & 3) != 0)
            return 0;
    }

    return 1;
}

int32_t DateTime::DateToDays(int32_t year, int32_t month, int32_t day) const
{
    int32_t shiftedYear = year - 2000;
    int32_t shiftedMonth;
    int32_t leapAdjustment;

    if (month < 3)
    {
        shiftedYear = year - 2001;
        shiftedMonth = month + 9;
    }
    else
    {
        shiftedMonth = month - 3;
    }

    leapAdjustment = shiftedYear < 0 ? static_cast<int32_t>(IsLeapYear(shiftedYear)) : 1;

    return ((shiftedYear / 100) * 146097) / 4
        + ((shiftedYear % 100) * 1461) / 4
        + (shiftedMonth * 153 + 2) / 5
        + (day - 1)
        + 59
        + leapAdjustment;
}

void DateTime::DaysToDate(int32_t days)
{
    int32_t shiftedDays = days + 2520114;
    int32_t century = (shiftedDays * 4) / 146097;
    int32_t dayOfCentury = shiftedDays - (century * 146097 + 3) / 4;
    int32_t yearOfCentury = (dayOfCentury * 4000 + 4000) / 1461001;
    int32_t dayOfYear = dayOfCentury - (yearOfCentury * 1461) / 4 + 31;
    int32_t monthIndex = (dayOfYear * 80) / 2447;
    int32_t day = dayOfYear - (monthIndex * 2447) / 80;
    int32_t month = monthIndex + 2 - (monthIndex / 11) * 12;
    int32_t year = century * 100 - 4900 + yearOfCentury + monthIndex / 11;

    m_packed = (m_packed & 0x1ffff)
        | ((static_cast<uint32_t>(day) << 17) & 0x3e0000)
        | ((static_cast<uint32_t>(month) << 22) & 0x3c00000)
        | (static_cast<uint32_t>(year) << 26);
    m_flags = (m_flags & 0xffffff00u) | ((static_cast<uint32_t>(year) >> 6) & 0xff);
}

uint64_t DateTime::ToEpochTime() const
{
    int32_t days = DateToDays(static_cast<int32_t>(GetYear()),
        static_cast<int32_t>(GetMonth()), static_cast<int32_t>(GetDay()));

    return static_cast<uint64_t>(static_cast<int64_t>(days) * 86400)
        + GetHour() * 3600
        + GetMinute() * 60
        + GetSecond();
}

bool DateTime::IsValid() const
{
    return ((m_flags >> 8) & 3) == 0;
}

bool DateTime::IsNever() const
{
    return ((static_cast<uint64_t>(m_flags & 0xfffffcffu) << 32) | m_packed)
        == static_cast<unsigned long long>(s_never);
}

void DateTime::Trace(uint64_t)
{
}

void DateTime::GetSystemTime(DateTime& dateTime)
{
    OSCalendarTime calendarTime;

    OSTicksToCalendarTime(OSGetTime(), &calendarTime);

    dateTime.m_flags = (dateTime.m_flags & 0xffffff00u)
        | (static_cast<uint32_t>(calendarTime.tm_year >> 6) & 0xff);
    dateTime.m_packed = (static_cast<uint32_t>(calendarTime.tm_sec) & 0x3f)
        | ((static_cast<uint32_t>(calendarTime.tm_min) & 0x3f) << 6)
        | ((static_cast<uint32_t>(calendarTime.tm_hour) & 0x1f) << 12)
        | ((static_cast<uint32_t>(calendarTime.tm_mday) & 0x1f) << 17)
        | ((static_cast<uint32_t>(calendarTime.tm_mon + 1) << 22) & 0x3c00000)
        | (static_cast<uint32_t>(calendarTime.tm_year) << 26);
}

DateTime DateTime::FromCustomEpochTime(int64_t epochTime, int32_t dayOffset)
{
    DateTime dateTime;
    uint64_t seconds = static_cast<uint64_t>(epochTime);

    dateTime.DaysToDate(static_cast<int32_t>(epochTime / 86400) + dayOffset);
    dateTime.m_packed = (dateTime.m_packed & 0xfffe0000u)
        | ((static_cast<uint32_t>((seconds / 3600) % 24) & 0x1f) << 12)
        | ((static_cast<uint32_t>((seconds / 60) % 60) & 0x3f) << 6)
        | (static_cast<uint32_t>(seconds % 60) & 0x3f);

    return dateTime;
}

DateTime DateTime::FromEpochTime(int64_t epochTime)
{
    return FromCustomEpochTime(epochTime, 0);
}

}
}
