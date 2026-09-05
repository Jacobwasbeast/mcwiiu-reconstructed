#include "game/command/ToggleDownfallCommand.hpp"

#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"

class MinecraftServer
{
public:
    static MinecraftServer* sInstance;
    Level* getLevel(int dimension);
};

ToggleDownfallCommand::~ToggleDownfallCommand()
{
}

EGameCommand ToggleDownfallCommand::getId()
{
    return eGameCommand_ToggleDownfall;
}

int ToggleDownfallCommand::getPermissionLevel()
{
    return 2;
}

void ToggleDownfallCommand::doToggleDownfall()
{
    LevelData levelData(MinecraftServer::sInstance->getLevel(0)->getLevelData());
    levelData.setRaining(!levelData.isRaining());
}

void ToggleDownfallCommand::execute(boost::shared_ptr<CommandSender> sender,
                                    arrayWithLength<unsigned char> data)
{
    doToggleDownfall();

    std::wstring message(L"commands.downfall.success");
    logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_Text,
                   NULL, 0, &message, 1);
}

boost::shared_ptr<GameCommandPacket> ToggleDownfallCommand::preparePacket()
{
    return boost::shared_ptr<GameCommandPacket>(
        new GameCommandPacket(eGameCommand_ToggleDownfall, arrayWithLength<unsigned char>()));
}
