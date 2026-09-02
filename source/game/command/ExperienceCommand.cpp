#include "game/command/ExperienceCommand.hpp"

ExperienceCommand::~ExperienceCommand()
{
}

EGameCommand ExperienceCommand::getId()
{
    return eGameCommand_Experience;
}

int ExperienceCommand::getPermissionLevel()
{
    return 2;
}

void ExperienceCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
}
