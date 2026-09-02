#include "game/entity/mob/EntityDataCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"

EntityDataCommand::~EntityDataCommand()
{
}

EGameCommand EntityDataCommand::getId()
{
    return eGameCommand_EntityData;
}

int EntityDataCommand::getPermissionLevel()
{
    return 2;
}

void EntityDataCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
}

boost::shared_ptr<GameCommandPacket> EntityDataCommand::preparePacket(unsigned char mode)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket());
}
