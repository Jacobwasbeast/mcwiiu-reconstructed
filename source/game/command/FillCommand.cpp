#include "game/command/FillCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/DataInputStream.hpp"

FillCommand::~FillCommand()
{
}

EGameCommand FillCommand::getId()
{
    return eGameCommand_Fill;
}

int FillCommand::getPermissionLevel()
{
    return 2;
}

void FillCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
}
