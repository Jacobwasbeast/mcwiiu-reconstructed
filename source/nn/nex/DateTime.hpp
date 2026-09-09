#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATETIME_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATETIME_HPP

#include <cstdint>

struct OSCalendarTime;

namespace nn
{
namespace nex
{

class DateTime
{
public:
    DateTime();
    DateTime(const DateTime& other);
    DateTime(std::uint16_t year, std::uint8_t month, std::uint8_t day,
        std::uint8_t hour, std::uint8_t minute, std::uint8_t second);
    explicit DateTime(const OSCalendarTime& calendarTime);

    explicit DateTime(std::uint64_t value)
        : m_flags(static_cast<std::uint32_t>(value >> 32))
        , m_packed(static_cast<std::uint32_t>(value))
    {
    }

    DateTime& operator=(const DateTime& other);

    operator unsigned long long();
    operator unsigned long long() const;

    bool operator==(const DateTime& other) const;
    bool operator!=(const DateTime& other) const;
    bool operator<(const DateTime& other) const;
    bool operator>(const DateTime& other) const;
    bool operator<=(const DateTime& other) const;
    bool operator>=(const DateTime& other) const;
    std::uint64_t operator-(const DateTime& other) const;

    std::uint32_t GetYear() const;
    std::uint32_t GetMonth() const;
    std::uint32_t GetDay() const;
    std::uint32_t GetHour() const;
    std::uint32_t GetMinute() const;
    std::uint8_t GetSecond() const;

    std::uint32_t IsLeapYear(std::int32_t year) const;
    std::int32_t DateToDays(std::int32_t year, std::int32_t month, std::int32_t day) const;
    void DaysToDate(std::int32_t days);

    std::uint64_t ToEpochTime() const;
    std::int64_t ToUnixEpochTime() const;
    OSCalendarTime ToCafeDateTime() const;

    bool IsValid() const;
    bool IsNever() const;
    void Trace(std::uint64_t flags);

    static void GetSystemTime(DateTime& dateTime);
    static DateTime FromEpochTime(std::int64_t epochTime);
    static DateTime FromUnixEpochTime(std::int64_t epochTime);
    static DateTime FromCustomEpochTime(std::int64_t epochTime, std::int32_t dayOffset);

private:
    std::uint32_t m_flags;
    std::uint32_t m_packed;
};

}
}

#endif
