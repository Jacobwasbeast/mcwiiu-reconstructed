#include "game/item/EffectCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/DataInputStream.hpp"

EGameCommand EffectCommand::getId()
{
    return eGameCommand_Effect;
}

int EffectCommand::getPermissionLevel()
{
    return 2;
}

void EffectCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream inputStream(&byteStream);
    std::wstring playerName = inputStream.readUTF();
    (void)playerName;

    byteStream.mBuffer.data = NULL;
    byteStream.mBuffer.length = 0;
    byteStream.mCount = 0;
    byteStream.mMark = 0;
    byteStream.mPosition = 0;
}
