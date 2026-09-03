#include "game/command/CommandDispatcher.hpp"

#include "game/command/CommandSender.hpp"

CommandDispatcher::CommandDispatcher()
    : boost::unordered_map<EGameCommand, Command*>(), mCommands()
{
}

CommandDispatcher::~CommandDispatcher()
{
    boost::unordered_set<Command*>::iterator it;
    for (it = mCommands.begin(); it != mCommands.end(); ++it)
    {
        delete *it;
    }
}

unsigned int CommandDispatcher::performCommand(boost::shared_ptr<CommandSender> sender,
                                               EGameCommand command,
                                               arrayWithLength<unsigned char> data)
{
    iterator it = find(command);
    if (it != end() && it->second != NULL)
    {
        Command* selected = it->second;
        if (selected->canExecute(sender))
        {
            selected->execute(sender, data);
        }
    }
    return 0;
}

Command* CommandDispatcher::addCommand(Command* command)
{
    if (command != NULL)
    {
        (*this)[command->getId()] = command;
        mCommands.insert(command);
    }
    return command;
}
