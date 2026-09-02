#ifndef GAME_COMMAND_COMMANDDISPATCHER_HPP
#define GAME_COMMAND_COMMANDDISPATCHER_HPP

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/command/Command.hpp"
#include "game/command/CommandSender.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class CommandDispatcher : public boost::unordered_map<EGameCommand, Command*>
{
public:
    CommandDispatcher();
    virtual ~CommandDispatcher();

    bool performCommand(boost::shared_ptr<CommandSender> sender, EGameCommand command, arrayWithLength<unsigned char> data);
    Command* addCommand(Command* command);

private:
    boost::unordered_set<Command*> ownedCommands;
};

#endif
