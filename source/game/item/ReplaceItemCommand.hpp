#ifndef GAME_ITEM_REPLACEITEMCOMMAND_HPP
#define GAME_ITEM_REPLACEITEMCOMMAND_HPP

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/command/CommandSender.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class ReplaceItemCommand : public Command
{
public:
    ReplaceItemCommand();
    virtual ~ReplaceItemCommand();

    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    static boost::shared_ptr<GameCommandPacket> preparePacket(unsigned char mode);
};

#endif
