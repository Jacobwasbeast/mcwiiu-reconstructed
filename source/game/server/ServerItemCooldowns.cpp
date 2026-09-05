#include "game/server/ServerItemCooldowns.hpp"

#include <boost/shared_ptr.hpp>

#include "game/network/ClientboundCooldownPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"

ServerItemCooldowns::ServerItemCooldowns(ServerPlayer* player)
    : ItemCooldowns()
    , mPlayer(player)
{
}

ServerItemCooldowns::~ServerItemCooldowns()
{
}

void ServerItemCooldowns::onCooldownStarted(Item* item, int duration)
{
    ItemCooldowns::onCooldownStarted(item, duration);

    if (mPlayer != 0 && mPlayer->connection != 0)
    {
        boost::shared_ptr<Packet> packet(new ClientboundCooldownPacket(item, duration));
        mPlayer->connection->send(packet);
    }
}

void ServerItemCooldowns::onCooldownEnded(Item* item)
{
    ItemCooldowns::onCooldownEnded(item);

    if (mPlayer != 0 && mPlayer->connection != 0)
    {
        boost::shared_ptr<Packet> packet(new ClientboundCooldownPacket(item, 0));
        mPlayer->connection->send(packet);
    }
}
