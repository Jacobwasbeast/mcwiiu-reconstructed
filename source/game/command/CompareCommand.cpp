#include "game/command/CompareCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/DataInputStream.hpp"

CompareCommand::~CompareCommand()
{
}

EGameCommand CompareCommand::getId()
{
    return eGameCommand_Compare;
}

int CompareCommand::getPermissionLevel()
{
    return 2;
}

void CompareCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
}
