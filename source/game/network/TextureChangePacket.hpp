#ifndef GAME_NETWORK_TEXTURECHANGEPACKET_HPP
#define GAME_NETWORK_TEXTURECHANGEPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;
class ServerGamePacketListenerImpl;

class TextureChangePacket : public Packet,
                            public boost::enable_shared_from_this<TextureChangePacket>
{
    friend class ServerGamePacketListenerImpl;

public:
    enum ETextureChangeType
    {
        eTextureChangeType_None = 0
    };

    TextureChangePacket();
    TextureChangePacket(boost::shared_ptr<Entity> entity, ETextureChangeType type,
                        const std::wstring& textureName);
    virtual ~TextureChangePacket();

    static boost::shared_ptr<Packet> create();

    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual int getPacketId();

private:
    int mEntityId;
    ETextureChangeType mChangeType;
    std::wstring mTextureName;
};

#endif
