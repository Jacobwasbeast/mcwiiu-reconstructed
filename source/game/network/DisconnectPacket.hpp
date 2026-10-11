#ifndef GAME_NETWORK_DISCONNECTPACKET_HPP
#define GAME_NETWORK_DISCONNECTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class DisconnectPacket : public Packet, public boost::enable_shared_from_this<DisconnectPacket>
{
public:
    enum eDisconnectReason
    {
        eDisconnectReason_None,
        eDisconnectReason_Quit,
        eDisconnectReason_ConnectionClosed,
        eDisconnectReason_LoginTimeout,
        eDisconnectReason_LoginFailed,
        eDisconnectReason_InternalError,
        eDisconnectReason_LevelLoadFailed,
        eDisconnectReason_KeepAliveTimeout,
        eDisconnectReason_Kicked,
        eDisconnectReason_InvalidMovement,
        eDisconnectReason_TimedOut,
        eDisconnectReason_SendQueueOverflow,
        eDisconnectReason_BadPacket,
        eDisconnectReason_GameFull,
        eDisconnectReason_HostOutOfDate,
        eDisconnectReason_ClientOutOfDate,
        eDisconnectReason_UnexpectedPacket,
        eDisconnectReason_Banned,
        eDisconnectReason_DuplicatePlayer,
        eDisconnectReason_HostSignedOut,
        eDisconnectReason_NotFriendsWithHost,
        eDisconnectReason_NotFriendsWithPlayer,
        eDisconnectReason_FriendsOnlyGame,
        eDisconnectReason_InviteOnlyGame,
        eDisconnectReason_PlayerBlocked,
        eDisconnectReason_NoMultiplayerPrivilege,
        eDisconnectReason_LoginRejected,
        eDisconnectReason_NoOnlinePrivilege,
        eDisconnectReason_NoUserContentPrivilege,
        eDisconnectReason_NetworkError,
        eDisconnectReason_AgeRestricted,
        eDisconnectReason_StorageError,
        eDisconnectReason_ContentNotOwned
    };

    DisconnectPacket();
    DisconnectPacket(eDisconnectReason reason);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new DisconnectPacket());
    }

    eDisconnectReason reason;
};

#endif
