#include "game/server/ServerCommandDispatcher.hpp"

#include "game/command/Command.hpp"
#include "game/command/ExperienceCommand.hpp"
#include "game/command/GameDifficultyCommand.hpp"
#include "game/command/KillCommand.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"

#include <vector>

extern MinecraftServer* gMinecraftServer;

class CheckpointCommand : public Command
{
public:
    virtual ~CheckpointCommand();
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender>, arrayWithLength<unsigned char>);
};

class GameModeCommand : public Command
{
public:
    virtual ~GameModeCommand();
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender>, arrayWithLength<unsigned char>);
};

class SetSpawnpointCommand : public Command
{
public:
    virtual ~SetSpawnpointCommand();
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender>, arrayWithLength<unsigned char>);
};

class TimeCommand : public Command
{
public:
    virtual ~TimeCommand();
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender>, arrayWithLength<unsigned char>);
};

class GiveItemCommand : public Command
{
public:
    virtual ~GiveItemCommand();
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender>, arrayWithLength<unsigned char>);
};

class DefaultGameModeCommand : public Command
{
public:
    DefaultGameModeCommand();
    virtual EGameCommand getId();
    virtual void execute(boost::shared_ptr<CommandSender>, arrayWithLength<unsigned char>);
};

class EnchantItemCommand : public Command
{
public:
    EnchantItemCommand();
    virtual EGameCommand getId();
    virtual void execute(boost::shared_ptr<CommandSender>, arrayWithLength<unsigned char>);
};

class ToggleDownfallCommand : public Command
{
public:
    ToggleDownfallCommand();
    virtual EGameCommand getId();
    virtual void execute(boost::shared_ptr<CommandSender>, arrayWithLength<unsigned char>);
};

class TeleportCommand : public Command
{
public:
    TeleportCommand();
    virtual EGameCommand getId();
    virtual void execute(boost::shared_ptr<CommandSender>, arrayWithLength<unsigned char>);
};

class SetDefaultSpawnpointCommand : public Command
{
public:
    SetDefaultSpawnpointCommand();
    virtual EGameCommand getId();
    virtual void execute(boost::shared_ptr<CommandSender>, arrayWithLength<unsigned char>);
};

class WeatherCommand : public Command
{
public:
    WeatherCommand();
    virtual EGameCommand getId();
    virtual void execute(boost::shared_ptr<CommandSender>, arrayWithLength<unsigned char>);
};

class ChangeCollisionTypeCommand : public Command
{
public:
    ChangeCollisionTypeCommand();
    virtual EGameCommand getId();
    virtual void execute(boost::shared_ptr<CommandSender>, arrayWithLength<unsigned char>);
};

ServerCommandDispatcher::ServerCommandDispatcher()
    : CommandDispatcher(), AdminLogCommand()
{
    addCommand(new TimeCommand());
    addCommand(new GameModeCommand());
    addCommand(new DefaultGameModeCommand());
    addCommand(new KillCommand());
    addCommand(new ToggleDownfallCommand());
    addCommand(new ExperienceCommand());
    addCommand(new TeleportCommand());
    addCommand(new GiveItemCommand());
    addCommand(new EnchantItemCommand());
    addCommand(new GameDifficultyCommand());
    addCommand(new SetDefaultSpawnpointCommand());
    addCommand(new WeatherCommand());
    addCommand(new SetSpawnpointCommand());
    addCommand(new CheckpointCommand());
    addCommand(new ChangeCollisionTypeCommand());
    Command::setLogger(this);
}

ServerCommandDispatcher::~ServerCommandDispatcher()
{
}

void ServerCommandDispatcher::logAdminCommand(boost::shared_ptr<CommandSender> sender,
                                              ClientboundChatPacket::EChatPacketMessage message,
                                              int* intParams, unsigned int numIntParams,
                                              std::wstring* stringParams, unsigned int numStringParams)
{
    PlayerList* players = gMinecraftServer->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> >* playerList = players->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it;
    for (it = playerList->begin(); it != playerList->end(); ++it)
    {
        if (message == ClientboundChatPacket::eChatPacketMessage_GameModeSuccessSelf)
        {
            if ((*it)->getId() != *intParams)
            {
                (*it)->sendMessage(ClientboundChatPacket::eChatPacketMessage_GameModeSuccessOther,
                                   intParams, numIntParams, stringParams, numStringParams);
                continue;
            }
        }
        (*it)->sendMessage(message, intParams, numIntParams, stringParams, numStringParams);
    }
}
