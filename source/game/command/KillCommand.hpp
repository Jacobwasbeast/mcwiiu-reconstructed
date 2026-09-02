#ifndef GAME_COMMAND_KILLCOMMAND_HPP
#define GAME_COMMAND_KILLCOMMAND_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/command/CommandSender.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class KillCommand : public Command
{
public:
    virtual ~KillCommand();

    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    static boost::shared_ptr<GameCommandPacket> preparePacket();

private:
    bool isValidWildcardPlayerArgument(std::wstring playerName, int argumentIndex);
};

#endif
