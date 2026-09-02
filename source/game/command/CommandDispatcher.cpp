#include "game/command/CommandDispatcher.hpp"

#include "game/command/Command.hpp"
#include "game/world/biome/arrayWithLength.hpp"

CommandDispatcher::CommandDispatcher()
{
}

CommandDispatcher::~CommandDispatcher()
{
    boost::unordered_set<Command*>::iterator it;
    for (it = ownedCommands.begin(); it != ownedCommands.end(); ++it)
    {
        delete *it;
    }
}

bool CommandDispatcher::performCommand(boost::shared_ptr<CommandSender> sender, EGameCommand command, arrayWithLength<unsigned char> data)
{
    boost::unordered_map<EGameCommand, Command*>::iterator it = find(command);
    if (it != end())
    {
        Command* handler = it->second;
        if (handler->canExecute(sender))
        {
            handler->execute(sender, data);
        }
    }

    return false;
}

Command* CommandDispatcher::addCommand(Command* command)
{
    (*this)[command->getId()] = command;
    ownedCommands.insert(command);

    return command;
}
