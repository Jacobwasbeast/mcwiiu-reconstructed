#include "game/core/Stat.hpp"

#include "game/core/DecimalFormat.hpp"

#include <utility>
#include <wchar.h>

template <class T>
std::wstring _toString(T value);

std::vector<Stat*>* Stat::stats = new std::vector<Stat*>();
boost::unordered_map<int, Stat*>* Stat::statsById = new boost::unordered_map<int, Stat*>();
StatFormatter* Stat::defaultFormatter = NULL;
StatFormatter* Stat::timeFormatter = NULL;
StatFormatter* Stat::distanceFormatter = NULL;
DecimalFormat* Stat::decimalFormat = NULL;

Stat::Stat(int id, const std::wstring& name, StatFormatter* formatter)
    : id(id),
      name(name),
      formatter(formatter)
{
    _init();
}

Stat::Stat(int id, const std::wstring& name)
    : id(id),
      name(name),
      formatter(defaultFormatter)
{
    _init();
}

Stat::~Stat() = default;

void Stat::_init()
{
    extended = false;
    biomeStat = false;
    awardLocallyOnly = false;
    armourStat = false;
    woolStat = false;
}

Stat* Stat::postConstruct(bool addToList)
{
    if (addToList)
    {
        stats->push_back(this);
    }

    statsById->insert(std::pair<int, Stat*>(id, this));

    return this;
}

bool Stat::isAchievement()
{
    return false;
}

bool Stat::isWoolStat()
{
    return woolStat;
}

bool Stat::isBiomeStat()
{
    return biomeStat;
}

bool Stat::isArmourStat()
{
    return armourStat;
}

void Stat::handleParamBlob(boost::shared_ptr<LocalPlayer> player,
                           arrayWithLength<unsigned char> params)
{
}

void Stat::setWoolStat(bool woolStat)
{
    this->woolStat = woolStat;
}

void Stat::setBiomeStat(bool biomeStat)
{
    this->biomeStat = biomeStat;
}

void Stat::setArmourStat(bool armourStat)
{
    this->armourStat = armourStat;
}

void Stat::setExtended()
{
    extended = true;
}


void Stat::setAwardLocallyOnly()
{
    awardLocallyOnly = true;
}

bool Stat::isAwardedFromServer() const
{
    return !awardLocallyOnly;
}

std::wstring Stat::format(int value)
{
    return formatter->format(value);
}

Stat::DefaultFormat::~DefaultFormat()
{
}

std::wstring Stat::DefaultFormat::format(int value)
{
    wchar_t buffer[256];

    swprintf(buffer, 256, L"%d", value);

    return std::wstring(buffer);
}

Stat::TimeFormatter::~TimeFormatter()
{
}

std::wstring Stat::TimeFormatter::format(int value)
{
    double seconds = (double)value / 20.0;
    double minutes = seconds / 60.0;
    double hours = minutes / 60.0;
    double days = hours / 24.0;
    double years = days / 365.0;

    if (years > 0.5)
    {
        return Stat::decimalFormat->format(years) + L" y";
    }
    else if (days > 0.5)
    {
        return Stat::decimalFormat->format(days) + L" d";
    }
    else if (hours > 0.5)
    {
        return Stat::decimalFormat->format(hours) + L" h";
    }
    else if (minutes > 0.5)
    {
        return Stat::decimalFormat->format(minutes) + L" m";
    }
    else
    {
        return _toString(seconds) + L" s";
    }
}

Stat::DistanceFormatter::~DistanceFormatter()
{
}

std::wstring Stat::DistanceFormatter::format(int value)
{
    double centimetres = (double)value;
    double metres = centimetres / 100.0;
    double kilometres = metres / 1000.0;

    if (kilometres > 0.5)
    {
        return Stat::decimalFormat->format(kilometres) + L" km";
    }
    else if (metres > 0.5)
    {
        return Stat::decimalFormat->format(metres) + L" m";
    }
    else
    {
        return _toString(value) + L" cm";
    }
}
