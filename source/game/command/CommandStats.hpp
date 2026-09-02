#ifndef GAME_COMMAND_COMMANDSTATS_HPP
#define GAME_COMMAND_COMMANDSTATS_HPP

#include <string>

#include <boost/shared_ptr.hpp>

class CommandSender;
class CompoundTag;
class MinecraftServer;

class CommandStats
{
public:
    enum CommandStatType
    {
        SUCCESS_COUNT,
        AFFECTED_BLOCKS,
        AFFECTED_ENTITIES,
        AFFECTED_ITEMS,
        QUERY_RESULT,
        NUM_COMMAND_STAT_TYPES
    };

    CommandStats();
    ~CommandStats();

    void setCommandStats(MinecraftServer* server, boost::shared_ptr<CommandSender> sender,
                         CommandStatType statType, int amount);
    void setCommandStats(boost::shared_ptr<CommandSender> sender, CommandStatType statType, int amount);

    bool hasStats(CommandStatType statType);

    void cloneFrom(CommandStats* other);

    void read(CompoundTag* tag);
    void write(CompoundTag* tag);

    static void assignStatsSetting(CommandStats* stats, CommandStatType statType,
                                   std::wstring* name, std::wstring* objective);
    static void unsetStatsSetting(CommandStats* stats, CommandStatType statType);

private:
    std::wstring* names;
    std::wstring* objectives;
};

#endif
