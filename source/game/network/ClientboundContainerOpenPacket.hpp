#ifndef GAME_NETWORK_CLIENTBOUNDCONTAINEROPENPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDCONTAINEROPENPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <stdint.h>
#include <string>

#include "game/core/InteractionObject.hpp"
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundContainerOpenPacket : public Packet,
                                       public boost::enable_shared_from_this<ClientboundContainerOpenPacket>
{
public:
    ClientboundContainerOpenPacket();
    ClientboundContainerOpenPacket(int containerId, int type, InteractionObject::NameData data);
    ClientboundContainerOpenPacket(int containerId, int type, InteractionObject::NameData data, int size);
    ClientboundContainerOpenPacket(int containerId, int type, InteractionObject::NameData data, int size, int entityId);
    virtual ~ClientboundContainerOpenPacket();

    void _init(int containerId, int type, InteractionObject::NameData data, int size, int entityId);

    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();

    int getContainerId();
    int getType();
    uint32_t hasCustomName();
    std::wstring getTitle();
    int getSize();
    int getEntityId();
    uint32_t isContainer();
    virtual int getPacketId();

    static boost::shared_ptr<Packet> create();

private:
    int mContainerId;
    int mType;
    int mSize;
    bool mHasCustomName;
    int mEntityId;
    InteractionObject::NameData mNameData;
};

#endif
