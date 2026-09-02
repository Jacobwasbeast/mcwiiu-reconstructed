#include "game/command/SayCommand.hpp"

SayCommand::~SayCommand()
{
}

EGameCommand SayCommand::getId()
{
    return eGameCommand_Say;
}

int SayCommand::getPermissionLevel()
{
    return 1;
}

void SayCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
}
