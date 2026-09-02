#include "game/command/CommandStats.hpp"

#include "game/data/CompoundTag.hpp"

static const std::wstring COMMAND_STAT_TYPE_NAMES[CommandStats::NUM_COMMAND_STAT_TYPES] =
{
    L"SuccessCount",
    L"AffectedBlocks",
    L"AffectedEntities",
    L"AffectedItems",
    L"QueryResult"
};

static std::wstring sCommandStatValues[CommandStats::NUM_COMMAND_STAT_TYPES];

CommandStats::CommandStats()
    : names(NULL)
    , objectives(NULL)
{
    for (int statType = 0; statType < NUM_COMMAND_STAT_TYPES; ++statType)
    {
        sCommandStatValues[statType] = std::wstring();
    }
}

CommandStats::~CommandStats()
{
}

void CommandStats::setCommandStats(MinecraftServer* server, boost::shared_ptr<CommandSender> sender,
                                   CommandStatType statType, int amount)
{
    setCommandStats(sender, statType, amount);
}

void CommandStats::write(CompoundTag* tag)
{
    CompoundTag* statsTag = new CompoundTag();

    for (int statType = 0; statType < NUM_COMMAND_STAT_TYPES; ++statType)
    {
        std::wstring name = names[statType];
        std::wstring objective = objectives[statType];

        if (name != COMMAND_STAT_TYPE_NAMES[statType] + L"Name" &&
            objective != COMMAND_STAT_TYPE_NAMES[statType] + L"Objective")
        {
            statsTag->putString(COMMAND_STAT_TYPE_NAMES[statType] + L"Name", name);
            statsTag->putString(COMMAND_STAT_TYPE_NAMES[statType] + L"Objective", objective);
        }
    }

    if (!statsTag->isEmpty())
    {
        tag->put(L"CommandStats", statsTag);
    }
}
