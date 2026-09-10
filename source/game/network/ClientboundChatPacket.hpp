#ifndef GAME_NETWORK_CLIENTBOUNDCHATPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDCHATPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/FlexiblePacket.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class Level;
class Packet;
class PacketListener;
class ClientPacketListener;

class ClientboundChatPacket : public FlexiblePacket, public boost::enable_shared_from_this<ClientboundChatPacket>
{
public:
    enum EChatPacketMessage
    {
        eChatPacketMessage_Text = 0,
        eChatPacketMessage_BedNotValid = 3,
        eChatPacketMessage_TimeSuccess = 99,
        eChatPacketMessage_DefaultSpawnpointSuccess = 102,
        eChatPacketMessage_DefaultSpawnpointNotValid = 103,
        eChatPacketMessage_SpawnpointSuccess = 104,
        eChatPacketMessage_SpawnpointNotValid = 105,
        eChatPacketMessage_GameModeSuccessSelf = 109,
        eChatPacketMessage_GameModeSuccessOther = 110,
        eChatPacketMessage_GameModeChanged = 111,
        eChatPacketMessage_ContainerLocked = 156
    };

    ClientboundChatPacket();
    ClientboundChatPacket(EChatPacketMessage message, int entityType);
    ClientboundChatPacket(EChatPacketMessage message, int* intParams, unsigned int numIntParams, std::wstring* stringParams, unsigned int numStringParams);
    ClientboundChatPacket(const std::wstring& text, EChatPacketMessage message, int entityType);
    ClientboundChatPacket(const std::wstring& text, EChatPacketMessage message, boost::shared_ptr<Entity> entity, const std::wstring& entityName);
    ClientboundChatPacket(const std::wstring& text, EChatPacketMessage message, boost::shared_ptr<Entity> entity, const std::wstring& entityName, const std::wstring& killerName);
    virtual ~ClientboundChatPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    std::wstring getMobCustomName(Level* level);

    EChatPacketMessage getMessage() const { return message; }

    static boost::shared_ptr<Packet> create();

private:
    friend class ClientPacketListener;
    friend struct ServerPlayer;
    EChatPacketMessage message;
};

#endif
