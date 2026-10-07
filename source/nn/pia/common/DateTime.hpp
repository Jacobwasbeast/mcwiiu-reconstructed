#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_DATE_TIME_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_DATE_TIME_HPP

namespace nn { namespace pia { namespace common {

class DateTime
{
public:
    DateTime()
        : m_year(0), m_month(0), m_day(0), m_hour(0), m_minute(0), m_second(0), m_unknown7(0)
    {
    }

    void Clear()
    {
        m_year = 0;
        m_month = 0;
        m_day = 0;
        m_hour = 0;
        m_minute = 0;
        m_second = 0;
        m_unknown7 = 0;
    }

    DateTime& operator=(const DateTime& other);

    virtual ~DateTime()
    {
    }

private:
    unsigned short m_year;
    unsigned char m_month;
    unsigned char m_day;
    unsigned char m_hour;
    unsigned char m_minute;
    unsigned char m_second;
    unsigned char m_unknown7;
};

inline DateTime& DateTime::operator=(const DateTime& other)
{
    m_year = other.m_year;
    m_month = other.m_month;
    m_day = other.m_day;
    m_hour = other.m_hour;
    m_minute = other.m_minute;
    m_second = other.m_second;
    m_unknown7 = other.m_unknown7;
    return *this;
}

}}}

#endif
