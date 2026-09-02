#include "game/item/ReplaceItemCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/DataInputStream.hpp"

ReplaceItemCommand::~ReplaceItemCommand()
{
}

EGameCommand ReplaceItemCommand::getId()
{
    return eGameCommand_ReplaceItem;
}

int ReplaceItemCommand::getPermissionLevel()
{
    return 2;
}

void ReplaceItemCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
}
