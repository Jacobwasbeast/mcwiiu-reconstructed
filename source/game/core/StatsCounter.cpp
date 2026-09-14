#include "game/core/StatsCounter.hpp"

#include "game/core/Stat.hpp"
#include "game/core/Stats.hpp"
#include "platform/wiiu/CProfile.hpp"

#include <cstring>

extern CProfile* gProfile;

namespace
{
    boost::unordered_map<Stat*, boost::unordered_map<unsigned int, StatsCounter::LEADERBOARD_FLAG> > gLeaderboardBoards;

    void putValues(boost::unordered_map<unsigned int, unsigned int>& values, unsigned int a, unsigned int b,
                   unsigned int c, unsigned int d)
    {
        values[0] = a;
        values[1] = b;
        values[2] = c;
        values[3] = d;
    }

    void copyValues(const boost::unordered_map<unsigned int, unsigned int>& values, unsigned char* destination,
                    bool large)
    {
        if (large)
        {
            unsigned int i;
            for (i = 0; i < 4; ++i)
            {
                unsigned int value = 0;
                boost::unordered_map<unsigned int, unsigned int>::const_iterator it = values.find(i);
                if (it != values.end())
                {
                    value = it->second;
                }
                std::memcpy(destination + i * 4, &value, sizeof(value));
            }
        }
        else
        {
            unsigned int i;
            for (i = 0; i < 4; ++i)
            {
                unsigned short value = 0;
                boost::unordered_map<unsigned int, unsigned int>::const_iterator it = values.find(i);
                if (it != values.end())
                {
                    value = static_cast<unsigned short>(it->second);
                }
                std::memcpy(destination + i * 2, &value, sizeof(value));
            }
        }
    }
}

StatsCounter::StatsCounter()
    : mSavePending(0), mSaveDelay(0), mLeaderboardPending(), mLeaderboardDelay(0)
{
}

void StatsCounter::setupStatBoards()
{
    if (Stat::stats == NULL)
    {
        return;
    }

    std::vector<Stat*>::const_iterator it;
    for (it = Stat::stats->begin(); it != Stat::stats->end(); ++it)
    {
        boost::unordered_map<unsigned int, LEADERBOARD_FLAG>& board = gLeaderboardBoards[*it];
        board[0] = eLeaderboard0;
        board[1] = eLeaderboard1;
        board[2] = eLeaderboard2;
        board[3] = eLeaderboard3;
    }
}

unsigned int StatsCounter::isLargeStat(Stat* stat)
{
    static Stat** largeStats[] = {
        &Stats::walkOneM, &Stats::swimOneM, &Stats::fallOneM, &Stats::climbOneM,
        &Stats::minecartOneM, &Stats::boatOneM, &Stats::pigOneM, &Stats::portalsUsed
    };
    for (unsigned int i = 0; i < sizeof(largeStats) / sizeof(largeStats[0]); ++i)
    {
        if (*largeStats[i] == stat)
        {
            return 1;
        }
    }
    return 0;
}

unsigned long long StatsCounter::award(Stat* stat, ESubTypes subtype, unsigned int amount, bool fromServer)
{
    if (stat == NULL)
    {
        return 0;
    }

    if (stat->isAchievement())
    {
        subtype = eSubtype0;
    }

    boost::unordered_map<Stat*, boost::unordered_map<unsigned int, unsigned int> >::iterator statIt = find(stat);
    if (statIt == end())
    {
        boost::unordered_map<unsigned int, unsigned int> values;
        putValues(values, 0, 0, 0, 0);
        values[static_cast<unsigned int>(subtype)] = amount;
        (*this)[stat] = values;
    }
    else if (!fromServer)
    {
        unsigned int& value = statIt->second[static_cast<unsigned int>(subtype)];
        value += amount;
        if (value < amount)
        {
            value = 0xffffffffu;
        }
        if (value > 0xffffu && !isLargeStat(stat))
        {
            value = 0xffffu;
        }
    }

    mSavePending = 1;
    boost::unordered_map<Stat*, boost::unordered_map<unsigned int, LEADERBOARD_FLAG> >::iterator boardIt =
        gLeaderboardBoards.find(stat);
    if (boardIt != gLeaderboardBoards.end())
    {
        LEADERBOARD_FLAG flag = boardIt->second[static_cast<unsigned int>(subtype)];
        mLeaderboardPending[static_cast<unsigned int>(flag)] = true;
        if (mLeaderboardDelay == 0)
        {
            mLeaderboardDelay = 9000;
        }
    }
    return 0;
}

int StatsCounter::getTotalValue(Stat* stat)
{
    boost::unordered_map<Stat*, boost::unordered_map<unsigned int, unsigned int> >::const_iterator statIt = find(stat);
    if (statIt == end())
    {
        return 0;
    }
    int total = 0;
    boost::unordered_map<unsigned int, unsigned int>::const_iterator it;
    for (it = statIt->second.begin(); it != statIt->second.end(); ++it)
    {
        total += static_cast<int>(it->second);
    }
    return total;
}

unsigned long long StatsCounter::save(int, bool, unsigned char* data)
{
    if (data != NULL)
    {
        std::memset(data + 0xcc, 0, 0x6cc);
    }
    if (data != NULL && Stat::stats != NULL)
    {
        unsigned char* cursor = data + 0xdd;
        std::vector<Stat*>::const_iterator it;
        for (it = Stat::stats->begin(); it != Stat::stats->end(); ++it)
        {
            boost::unordered_map<Stat*, boost::unordered_map<unsigned int, unsigned int> >::const_iterator statIt = find(*it);
            bool large = isLargeStat(*it) != 0;
            if (statIt != end())
            {
                copyValues(statIt->second, cursor, large);
            }
            cursor += large ? 16 : 8;
        }
    }
    mSavePending = 0;
    mSaveDelay = 0x708;
    return 0;
}

void StatsCounter::writeStats()
{
    if (gProfile != NULL)
        (void)gProfile->IsFullVersion();
}

void StatsCounter::flushLeaderboards()
{
    writeStats();
    mLeaderboardPending.clear();
}

void StatsCounter::tick(int profile)
{
    if (mSaveDelay > 0)
    {
        --mSaveDelay;
    }
    if (mSavePending != 0 && mSaveDelay == 0)
    {
        save(profile, false, NULL);
    }
    if (mLeaderboardDelay > 0)
    {
        --mLeaderboardDelay;
        if (mLeaderboardDelay == 0)
        {
            flushLeaderboards();
        }
    }
}

void StatsCounter::parse(void* data)
{
    if (data == NULL || Stat::stats == NULL)
    {
        return;
    }
    clear();
    unsigned char* cursor = static_cast<unsigned char*>(data) + 0xdd;
    std::vector<Stat*>::const_iterator it;
    for (it = Stat::stats->begin(); it != Stat::stats->end(); ++it)
    {
        boost::unordered_map<unsigned int, unsigned int> values;
        if (isLargeStat(*it))
        {
            unsigned int a, b, c, d;
            std::memcpy(&a, cursor, 4); std::memcpy(&b, cursor + 4, 4);
            std::memcpy(&c, cursor + 8, 4); std::memcpy(&d, cursor + 12, 4);
            putValues(values, a, b, c, d);
            cursor += 16;
        }
        else
        {
            unsigned short a, b, c, d;
            std::memcpy(&a, cursor, 2); std::memcpy(&b, cursor + 2, 2);
            std::memcpy(&c, cursor + 4, 2); std::memcpy(&d, cursor + 6, 2);
            putValues(values, a, b, c, d);
            cursor += 8;
        }
        if (values[0] != 0 || values[1] != 0 || values[2] != 0 || values[3] != 0)
        {
            (*this)[*it] = values;
        }
    }
}

void StatsCounter::parseOld(unsigned short, void* data)
{
    unsigned char* cursor = static_cast<unsigned char*>(data) + 0xcc;
    std::vector<Stat*>::const_iterator it;
    for (it = Stat::stats->begin(); it != Stat::stats->end(); ++it)
    {
        Stat* stat = *it;
        if (!stat->isAchievement())
        {
            if (isLargeStat(stat))
            {
                unsigned int a, b, c, d;
                std::memcpy(&a, cursor, 4);
                std::memcpy(&b, cursor + 4, 4);
                std::memcpy(&c, cursor + 8, 4);
                std::memcpy(&d, cursor + 12, 4);
                if (a != 0 || b != 0 || c != 0 || d != 0)
                {
                    boost::unordered_map<unsigned int, unsigned int> values;
                    putValues(values, a, b, c, d);
                    insert(std::make_pair(stat, values));
                }
                cursor += 16;
            }
            else
            {
                unsigned short a, b, c, d;
                std::memcpy(&a, cursor, 2);
                std::memcpy(&b, cursor + 2, 2);
                std::memcpy(&c, cursor + 4, 2);
                std::memcpy(&d, cursor + 6, 2);
                if (a != 0 || b != 0 || c != 0 || d != 0)
                {
                    boost::unordered_map<unsigned int, unsigned int> values;
                    putValues(values, a, b, c, d);
                    insert(std::make_pair(stat, values));
                }
                cursor += 8;
            }
        }
        else
        {
            unsigned short a;
            std::memcpy(&a, cursor, 2);
            if (a != 0)
            {
                boost::unordered_map<unsigned int, unsigned int> values;
                putValues(values, a, 0, 0, 0);
                insert(std::make_pair(stat, values));
            }
            cursor += 2;
        }
    }
}

void StatsCounter::clear()
{
    boost::unordered_map<Stat*, boost::unordered_map<unsigned int, unsigned int> >::clear();
    mLeaderboardPending.clear();
}

void StatsCounter::saveLeaderboards()
{
    writeStats();
    mLeaderboardPending.clear();
}
