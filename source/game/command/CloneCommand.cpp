#include "game/command/CloneCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/DataInputStream.hpp"

CloneCommand::~CloneCommand()
{
}

EGameCommand CloneCommand::getId()
{
    return eGameCommand_Clone;
}

int CloneCommand::getPermissionLevel()
{
    return 2;
}

void CloneCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
}
