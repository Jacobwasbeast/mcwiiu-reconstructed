#include "game/world/block/SetBlockCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"

SetBlockCommand::~SetBlockCommand()
{
}

EGameCommand SetBlockCommand::getId()
{
    return eGameCommand_SetBlock;
}

int SetBlockCommand::getPermissionLevel()
{
    return 2;
}

void SetBlockCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
}

boost::shared_ptr<GameCommandPacket> SetBlockCommand::preparePacket(unsigned char mode)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket());
}
