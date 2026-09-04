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
    DataInputStream stream(&byteStream);
}

boost::shared_ptr<GameCommandPacket> CloneCommand::preparePacket(unsigned char mode)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writeByte(mode);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_Clone, byteStream.toByteArray()));
}
