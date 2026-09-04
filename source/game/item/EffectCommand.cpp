#include "game/item/EffectCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/DataInputStream.hpp"

EffectCommand::~EffectCommand()
{
}

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
}

std::wstring EffectCommand::getUsage(CommandSender* sender)
{
    return L"commands.effect.usage";
}

std::vector<std::wstring> EffectCommand::getPlayerNames()
{
    std::vector<std::wstring> playerNames;
    playerNames.push_back(L"*");
    return playerNames;
}

bool EffectCommand::isValidWildcardPlayerArgument(std::wstring playerName, int argumentIndex)
{
    return argumentIndex == 0;
}
