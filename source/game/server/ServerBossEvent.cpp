#include "game/server/ServerBossEvent.hpp"

#include <map>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/network/ClientboundBossEventPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"

ServerBossEvent::ServerBossEvent(int nameId, const std::wstring& name,
                                 BossBarColor color, BossBarOverlay overlay)
    : BossEvent(Mth::createInsecureUUID(), nameId, name, color, overlay),
      visible(true)
{
}

ServerBossEvent::~ServerBossEvent()
{
}

void ServerBossEvent::broadcast(ClientboundBossEventPacket::Operation operation)
{
    if (!visible)
    {
        return;
    }

    boost::shared_ptr<Packet> packet(new ClientboundBossEventPacket(operation, this));
    std::map<boost::shared_ptr<ServerPlayer>, int>::iterator it;
    for (it = players.begin(); it != players.end(); ++it)
    {
        if (it->first->mConnection != 0)
        {
            it->first->mConnection->send(packet);
        }
    }
}

void ServerBossEvent::setPercent(float percent)
{
    if (percent != this->percent)
    {
        BossEvent::setPercent(percent);
        broadcast(ClientboundBossEventPacket::UPDATE_PROGRESS);
    }
}

void ServerBossEvent::setColor(BossBarColor color)
{
    if (color != this->color)
    {
        BossEvent::setColor(color);
        broadcast(ClientboundBossEventPacket::UPDATE_STYLE);
    }
}

void ServerBossEvent::setOverlay(BossBarOverlay overlay)
{
    if (overlay != this->overlay)
    {
        BossEvent::setOverlay(overlay);
        broadcast(ClientboundBossEventPacket::UPDATE_STYLE);
    }
}

BossEvent* ServerBossEvent::setDarkenScreen(bool darkenScreen)
{
    if (darkenScreen != this->darkenScreen)
    {
        BossEvent::setDarkenScreen(darkenScreen);
        broadcast(ClientboundBossEventPacket::UPDATE_PROPERTIES);
    }
    return this;
}

BossEvent* ServerBossEvent::setPlayBossMusic(bool playBossMusic)
{
    if (playBossMusic != this->playBossMusic)
    {
        BossEvent::setPlayBossMusic(playBossMusic);
        broadcast(ClientboundBossEventPacket::UPDATE_PROPERTIES);
    }
    return this;
}

BossEvent* ServerBossEvent::setCreateWorldFog(bool createWorldFog)
{
    if (createWorldFog != this->createWorldFog)
    {
        BossEvent::setCreateWorldFog(createWorldFog);
        broadcast(ClientboundBossEventPacket::UPDATE_PROPERTIES);
    }
    return this;
}

void ServerBossEvent::setName(const std::wstring& name)
{
    if (name != this->name)
    {
        BossEvent::setName(name);
        broadcast(ClientboundBossEventPacket::UPDATE_NAME);
    }
}

void ServerBossEvent::addPlayer(boost::shared_ptr<ServerPlayer> player, int party)
{
    std::pair<std::map<boost::shared_ptr<ServerPlayer>, int>::iterator, bool> result =
        players.insert(std::make_pair(player, party));
    if (!result.second)
    {
        result.first->second = party;
    }
    else if (visible && player->mConnection != 0)
    {
        boost::shared_ptr<Packet> packet(
            new ClientboundBossEventPacket(ClientboundBossEventPacket::ADD, this));
        player->mConnection->send(packet);
    }
}

void ServerBossEvent::removePlayer(boost::shared_ptr<ServerPlayer> player)
{
    std::map<boost::shared_ptr<ServerPlayer>, int>::iterator it = players.find(player);
    if (it != players.end())
    {
        players.erase(it);
        if (visible && player->mConnection != 0)
        {
            boost::shared_ptr<Packet> packet(
                new ClientboundBossEventPacket(ClientboundBossEventPacket::REMOVE, this));
            player->mConnection->send(packet);
        }
    }
}

void ServerBossEvent::removeAllPlayers()
{
    while (!players.empty())
    {
        removePlayer(players.begin()->first);
    }
}

void ServerBossEvent::setVisible(bool visible)
{
    if (visible != this->visible)
    {
        this->visible = visible;
        std::map<boost::shared_ptr<ServerPlayer>, int>::iterator it;
        ClientboundBossEventPacket::Operation operation =
            visible ? ClientboundBossEventPacket::ADD : ClientboundBossEventPacket::REMOVE;
        for (it = players.begin(); it != players.end(); ++it)
        {
            if (it->first->mConnection != 0)
            {
                boost::shared_ptr<Packet> packet(
                    new ClientboundBossEventPacket(operation, this));
                it->first->mConnection->send(packet);
            }
        }
    }
}

std::vector<boost::shared_ptr<ServerPlayer> > ServerBossEvent::getPlayers()
{
    std::vector<boost::shared_ptr<ServerPlayer> > result;
    std::map<boost::shared_ptr<ServerPlayer>, int>::iterator it;
    for (it = players.begin(); it != players.end(); ++it)
    {
        result.push_back(it->first);
    }
    return result;
}

std::map<boost::shared_ptr<ServerPlayer>, int>& ServerBossEvent::getPartyPlayers()
{
    return players;
}
