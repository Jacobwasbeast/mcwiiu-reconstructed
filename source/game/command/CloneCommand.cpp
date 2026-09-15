#include "game/command/CloneCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"

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
    DataInputStream inputStream(&byteStream);
    (void)sender;
    (void)inputStream;
}

