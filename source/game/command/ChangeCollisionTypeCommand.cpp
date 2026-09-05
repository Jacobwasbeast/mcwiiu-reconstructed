#include "game/command/ChangeCollisionTypeCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerList.hpp"

ChangeCollisionTypeCommand::~ChangeCollisionTypeCommand()
{
}

EGameCommand ChangeCollisionTypeCommand::getId()
{
    return eGameCommand_ChangeCollisionType;
}

int ChangeCollisionTypeCommand::getPermissionLevel()
{
    return 0;
}

void ChangeCollisionTypeCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);

    PlayerUID uid = stream.readPlayerUID();
    boost::shared_ptr<ServerPlayer> player = getPlayer(uid);
    int collisionType = stream.readInt();

    if (collisionType == 1)
    {
        player->SetSmallSize();
    }
    else if (collisionType == 2)
    {
        player->SetNoCollisionSize();
    }
    else
    {
        player->SetNormalSize();
    }
}

boost::shared_ptr<GameCommandPacket> ChangeCollisionTypeCommand::preparePacket(PlayerUID uid, int collisionType)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writePlayerUID(uid);
    stream.writeInt(collisionType);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_ChangeCollisionType, byteStream.toByteArray()));
}
