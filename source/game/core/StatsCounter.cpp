#include "game/core/StatsCounter.hpp"

#include "game/core/Stat.hpp"
#include "game/core/Stats.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"

#include <cstring>

extern CProfile* gProfile;
extern CStorage* gStorage;

boost::unordered_map<Stat*, boost::unordered_map<unsigned int, StatsCounter::LEADERBOARD_FLAG> > StatsCounter::statBoards;

StatsCounter::StatsCounter()
    : mSavePending(0), mSaveDelay(0), mLeaderboardPending(), mLeaderboardDelay(0)
{
}

void StatsCounter::setupStatBoards()
{
    statBoards[Stats::killsZombie][0] = eLeaderboard0;
    statBoards[Stats::killsZombie][1] = eLeaderboard1;
    statBoards[Stats::killsZombie][2] = eLeaderboard2;
    statBoards[Stats::killsZombie][3] = eLeaderboard3;

    statBoards[Stats::killsSkeleton][0] = eLeaderboard0;
    statBoards[Stats::killsSkeleton][1] = eLeaderboard1;
    statBoards[Stats::killsSkeleton][2] = eLeaderboard2;
    statBoards[Stats::killsSkeleton][3] = eLeaderboard3;

    statBoards[Stats::killsCreeper][0] = eLeaderboard0;
    statBoards[Stats::killsCreeper][1] = eLeaderboard1;
    statBoards[Stats::killsCreeper][2] = eLeaderboard2;
    statBoards[Stats::killsCreeper][3] = eLeaderboard3;

    statBoards[Stats::killsSpider][0] = eLeaderboard0;
    statBoards[Stats::killsSpider][1] = eLeaderboard1;
    statBoards[Stats::killsSpider][2] = eLeaderboard2;
    statBoards[Stats::killsSpider][3] = eLeaderboard3;

    statBoards[Stats::killsSpiderJockey][0] = eLeaderboard0;
    statBoards[Stats::killsSpiderJockey][1] = eLeaderboard1;
    statBoards[Stats::killsSpiderJockey][2] = eLeaderboard2;
    statBoards[Stats::killsSpiderJockey][3] = eLeaderboard3;

    statBoards[Stats::killsZombiePigman][0] = eLeaderboard0;
    statBoards[Stats::killsZombiePigman][1] = eLeaderboard1;
    statBoards[Stats::killsZombiePigman][2] = eLeaderboard2;
    statBoards[Stats::killsZombiePigman][3] = eLeaderboard3;

    statBoards[Stats::killsNetherZombiePigman][0] = eLeaderboard0;
    statBoards[Stats::killsNetherZombiePigman][1] = eLeaderboard1;
    statBoards[Stats::killsNetherZombiePigman][2] = eLeaderboard2;
    statBoards[Stats::killsNetherZombiePigman][3] = eLeaderboard3;

    statBoards[Stats::killsSlime][0] = eLeaderboard0;
    statBoards[Stats::killsSlime][1] = eLeaderboard1;
    statBoards[Stats::killsSlime][2] = eLeaderboard2;
    statBoards[Stats::killsSlime][3] = eLeaderboard3;

    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::dirt)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::dirt)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::dirt)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::dirt)]][3] = eLeaderboard7;

    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::cobblestone)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::cobblestone)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::cobblestone)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::cobblestone)]][3] = eLeaderboard7;

    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::sand)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::sand)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::sand)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::sand)]][3] = eLeaderboard7;

    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::stone)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::stone)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::stone)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::stone)]][3] = eLeaderboard7;

    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::gravel)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::gravel)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::gravel)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::gravel)]][3] = eLeaderboard7;

    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::clay)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::clay)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::clay)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::clay)]][3] = eLeaderboard7;

    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::obsidian)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::obsidian)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::obsidian)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::obsidian)]][3] = eLeaderboard7;

    statBoards[Stats::itemCollectedStats.data[Item::getId(Items::egg)]][0] = eLeaderboard12;
    statBoards[Stats::itemCollectedStats.data[Item::getId(Items::egg)]][1] = eLeaderboard13;
    statBoards[Stats::itemCollectedStats.data[Item::getId(Items::egg)]][2] = eLeaderboard14;
    statBoards[Stats::itemCollectedStats.data[Item::getId(Items::egg)]][3] = eLeaderboard15;

    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::wheat)]][0] = eLeaderboard12;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::wheat)]][1] = eLeaderboard13;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::wheat)]][2] = eLeaderboard14;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::wheat)]][3] = eLeaderboard15;

    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::brown_mushroom)]][0] = eLeaderboard12;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::brown_mushroom)]][1] = eLeaderboard13;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::brown_mushroom)]][2] = eLeaderboard14;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::brown_mushroom)]][3] = eLeaderboard15;

    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::reeds)]][0] = eLeaderboard12;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::reeds)]][1] = eLeaderboard13;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::reeds)]][2] = eLeaderboard14;
    statBoards[Stats::blockMinedStats.data[Block::getId(Blocks::reeds)]][3] = eLeaderboard15;

    statBoards[Stats::cowsMilked][0] = eLeaderboard12;
    statBoards[Stats::cowsMilked][1] = eLeaderboard13;
    statBoards[Stats::cowsMilked][2] = eLeaderboard14;
    statBoards[Stats::cowsMilked][3] = eLeaderboard15;

    statBoards[Stats::itemCollectedStats.data[Block::getId(Blocks::pumpkin)]][0] = eLeaderboard12;
    statBoards[Stats::itemCollectedStats.data[Block::getId(Blocks::pumpkin)]][1] = eLeaderboard13;
    statBoards[Stats::itemCollectedStats.data[Block::getId(Blocks::pumpkin)]][2] = eLeaderboard14;
    statBoards[Stats::itemCollectedStats.data[Block::getId(Blocks::pumpkin)]][3] = eLeaderboard15;

    statBoards[Stats::walkOneM][0] = eLeaderboard16;
    statBoards[Stats::walkOneM][1] = eLeaderboard17;
    statBoards[Stats::walkOneM][2] = eLeaderboard18;
    statBoards[Stats::walkOneM][3] = eLeaderboard19;

    statBoards[Stats::fallOneM][0] = eLeaderboard16;
    statBoards[Stats::fallOneM][1] = eLeaderboard17;
    statBoards[Stats::fallOneM][2] = eLeaderboard18;
    statBoards[Stats::fallOneM][3] = eLeaderboard19;

    statBoards[Stats::minecartOneM][0] = eLeaderboard16;
    statBoards[Stats::minecartOneM][1] = eLeaderboard17;
    statBoards[Stats::minecartOneM][2] = eLeaderboard18;
    statBoards[Stats::minecartOneM][3] = eLeaderboard19;

    statBoards[Stats::boatOneM][0] = eLeaderboard16;
    statBoards[Stats::boatOneM][1] = eLeaderboard17;
    statBoards[Stats::boatOneM][2] = eLeaderboard18;
    statBoards[Stats::boatOneM][3] = eLeaderboard19;

    statBoards[Stats::mg03BestTime][4] = eLeaderboard37;
    statBoards[Stats::mg03BestTime][5] = eLeaderboard38;
    statBoards[Stats::mg03BestTime][6] = eLeaderboard39;
    statBoards[Stats::mg03BestTime][7] = eLeaderboard40;
    statBoards[Stats::mg03BestTime][8] = eLeaderboard41;
    statBoards[Stats::mg03BestTime][9] = eLeaderboard42;
    statBoards[Stats::mg03BestTime][10] = eLeaderboard43;
    statBoards[Stats::mg03BestTime][11] = eLeaderboard44;
    statBoards[Stats::mg03BestTime][12] = eLeaderboard45;
    statBoards[Stats::mg03BestTime][13] = eLeaderboard46;
    statBoards[Stats::mg03BestTime][14] = eLeaderboard47;
    statBoards[Stats::mg03BestTime][15] = eLeaderboard48;

    statBoards[Stats::mg03BestTimeScoreChallenge][4] = eLeaderboard25;
    statBoards[Stats::mg03BestTimeScoreChallenge][5] = eLeaderboard26;
    statBoards[Stats::mg03BestTimeScoreChallenge][6] = eLeaderboard27;
    statBoards[Stats::mg03BestTimeScoreChallenge][7] = eLeaderboard28;
    statBoards[Stats::mg03BestTimeScoreChallenge][8] = eLeaderboard29;
    statBoards[Stats::mg03BestTimeScoreChallenge][8] = eLeaderboard30;
    statBoards[Stats::mg03BestTimeScoreChallenge][10] = eLeaderboard31;
    statBoards[Stats::mg03BestTimeScoreChallenge][11] = eLeaderboard32;
    statBoards[Stats::mg03BestTimeScoreChallenge][12] = eLeaderboard33;
    statBoards[Stats::mg03BestTimeScoreChallenge][13] = eLeaderboard34;
    statBoards[Stats::mg03BestTimeScoreChallenge][14] = eLeaderboard35;
    statBoards[Stats::mg03BestTimeScoreChallenge][15] = eLeaderboard36;

    statBoards[Stats::mg03Score][4] = eLeaderboard25;
    statBoards[Stats::mg03Score][5] = eLeaderboard26;
    statBoards[Stats::mg03Score][6] = eLeaderboard27;
    statBoards[Stats::mg03Score][7] = eLeaderboard28;
    statBoards[Stats::mg03Score][8] = eLeaderboard29;
    statBoards[Stats::mg03Score][9] = eLeaderboard30;
    statBoards[Stats::mg03Score][10] = eLeaderboard31;
    statBoards[Stats::mg03Score][11] = eLeaderboard32;
    statBoards[Stats::mg03Score][12] = eLeaderboard33;
    statBoards[Stats::mg03Score][13] = eLeaderboard34;
    statBoards[Stats::mg03Score][14] = eLeaderboard35;
    statBoards[Stats::mg03Score][15] = eLeaderboard36;

    statBoards[Stats::mg03RingCount][4] = eLeaderboard25;
    statBoards[Stats::mg03RingCount][5] = eLeaderboard26;
    statBoards[Stats::mg03RingCount][6] = eLeaderboard27;
    statBoards[Stats::mg03RingCount][7] = eLeaderboard28;
    statBoards[Stats::mg03RingCount][8] = eLeaderboard29;
    statBoards[Stats::mg03RingCount][8] = eLeaderboard30;
    statBoards[Stats::mg03RingCount][10] = eLeaderboard31;
    statBoards[Stats::mg03RingCount][11] = eLeaderboard32;
    statBoards[Stats::mg03RingCount][12] = eLeaderboard33;
    statBoards[Stats::mg03RingCount][13] = eLeaderboard34;
    statBoards[Stats::mg03RingCount][14] = eLeaderboard35;
    statBoards[Stats::mg03RingCount][15] = eLeaderboard36;
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
        values[0] = 0;
        values[1] = 0;
        values[2] = 0;
        values[3] = 0;
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
        statBoards.find(stat);
    if (boardIt != statBoards.end())
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

unsigned long long StatsCounter::save(int profile, bool force, unsigned char* data)
{
    if (!gProfile->IsFullVersion())
    {
        return 0;
    }

    bool ownBuffer = data == NULL;
    if (ownBuffer)
    {
        data = gStorage->GetGameDefinedProfileData(profile);
    }

    std::memset(data + 0xcc, 0, 0x6cc);

    if (Stat::stats != NULL)
    {
        unsigned long long woolBits = 0;
        unsigned int woolIndex = 0;
        unsigned long long biomeBits = 0;
        unsigned int biomeIndex = 0;
        unsigned char armourBits = 0;
        unsigned int armourIndex = 0;
        unsigned char* cursor = data + 0xdd;

        std::vector<Stat*>::const_iterator it;
        for (it = Stat::stats->begin(); it != Stat::stats->end(); ++it)
        {
            Stat* stat = *it;
            boost::unordered_map<Stat*, boost::unordered_map<unsigned int, unsigned int> >::iterator statIt = find(stat);

            if (stat->isAchievement())
            {
                continue;
            }
            else if (stat->isWoolStat())
            {
                if (statIt != end())
                {
                    boost::unordered_map<unsigned int, unsigned int>::const_iterator valueIt = statIt->second.find(0);
                    if (valueIt != statIt->second.end() && (valueIt->second & 1) != 0)
                    {
                        woolBits |= (1ULL << woolIndex);
                    }
                }
                ++woolIndex;
            }
            else if (stat->isBiomeStat())
            {
                if (statIt != end())
                {
                    boost::unordered_map<unsigned int, unsigned int>::const_iterator valueIt = statIt->second.find(0);
                    if (valueIt != statIt->second.end() && (valueIt->second & 1) != 0)
                    {
                        biomeBits |= (1ULL << biomeIndex);
                    }
                }
                ++biomeIndex;
            }
            else if (stat->isArmourStat())
            {
                if (statIt != end())
                {
                    boost::unordered_map<unsigned int, unsigned int>::const_iterator valueIt = statIt->second.find(0);
                    if (valueIt != statIt->second.end() && (valueIt->second & 1) != 0)
                    {
                        armourBits |= static_cast<unsigned char>(1u << armourIndex);
                    }
                }
                ++armourIndex;
            }
            else
            {
                bool large = isLargeStat(stat) != 0;
                if (statIt != end())
                {
                    unsigned int i;
                    for (i = 0; i < 4; ++i)
                    {
                        if (large)
                        {
                            unsigned int value = statIt->second[i];
                            std::memcpy(cursor + i * sizeof(value), &value, sizeof(value));
                        }
                        else
                        {
                            unsigned short value = static_cast<unsigned short>(statIt->second[i]);
                            std::memcpy(cursor + i * sizeof(value), &value, sizeof(value));
                        }
                    }
                }
                cursor += large ? 16 : 8;
            }
        }

        std::memcpy(data + 0xcc, &woolBits, sizeof(woolBits));
        std::memcpy(data + 0xd4, &biomeBits, sizeof(biomeBits));
        data[0xdc] = armourBits;
    }

    if (ownBuffer)
    {
        gStorage->WriteToProfile(profile, true, force);
    }

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
    unsigned char* bytes = static_cast<unsigned char*>(data);
    unsigned char* cursor = bytes + 0xdd;
    unsigned int woolIndex = 0;
    unsigned int biomeIndex = 0;
    unsigned int armourIndex = 0;
    boost::unordered_map<unsigned int, unsigned int> values;
    std::vector<Stat*>::const_iterator it;
    for (it = Stat::stats->begin(); it != Stat::stats->end(); ++it)
    {
        Stat* stat = *it;
        if (stat->isAchievement())
        {
            continue;
        }
        else if (stat->isWoolStat())
        {
            unsigned long long woolBits;
            std::memcpy(&woolBits, bytes + 0xcc, sizeof(woolBits));
            if (woolBits != 0)
            {
                values[0] = 0;
                values[0] |= static_cast<unsigned int>((woolBits >> woolIndex) & 1);
                values[1] = 0;
                values[2] = 0;
                values[3] = 0;
                insert(std::make_pair(stat, values));
            }
            ++woolIndex;
        }
        else if (stat->isBiomeStat())
        {
            unsigned long long biomeBits;
            std::memcpy(&biomeBits, bytes + 0xd4, sizeof(biomeBits));
            if (biomeBits != 0)
            {
                values[0] = 0;
                values[0] |= static_cast<unsigned int>((biomeBits >> biomeIndex) & 1);
                values[1] = 0;
                values[2] = 0;
                values[3] = 0;
                insert(std::make_pair(stat, values));
            }
            ++biomeIndex;
        }
        else if (stat->isArmourStat())
        {
            signed char armourBits = static_cast<signed char>(bytes[0xdc]);
            if (armourBits != 0)
            {
                values[0] = 0;
                values[0] |= (armourBits >> armourIndex) & 1;
                values[1] = 0;
                values[2] = 0;
                values[3] = 0;
                insert(std::make_pair(stat, values));
            }
            ++armourIndex;
        }
        else if (isLargeStat(stat))
        {
            unsigned int a, b, c, d;
            std::memcpy(&a, cursor, 4);
            std::memcpy(&b, cursor + 4, 4);
            std::memcpy(&c, cursor + 8, 4);
            std::memcpy(&d, cursor + 12, 4);
            if (a != 0 || b != 0 || c != 0 || d != 0)
            {
                values[0] = a;
                values[1] = b;
                values[2] = c;
                values[3] = d;
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
                values[0] = a;
                values[1] = b;
                values[2] = c;
                values[3] = d;
                insert(std::make_pair(stat, values));
            }
            cursor += 8;
        }
    }
}

void StatsCounter::parseOld(unsigned short, void* data)
{
    unsigned char* cursor = static_cast<unsigned char*>(data) + 0xcc;
    boost::unordered_map<unsigned int, unsigned int> values;
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
                    values[0] = a;
                    values[1] = b;
                    values[2] = c;
                    values[3] = d;
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
                    values[0] = a;
                    values[1] = b;
                    values[2] = c;
                    values[3] = d;
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
                values[0] = a;
                values[1] = 0;
                values[2] = 0;
                values[3] = 0;
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
