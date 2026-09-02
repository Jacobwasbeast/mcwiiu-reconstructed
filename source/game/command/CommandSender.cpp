#include "game/command/CommandSender.hpp"

CommandSender::~CommandSender()
{
}

boost::shared_ptr<Entity> CommandSender::getCommandSenderEntity()
{
    return boost::shared_ptr<Entity>();
}
