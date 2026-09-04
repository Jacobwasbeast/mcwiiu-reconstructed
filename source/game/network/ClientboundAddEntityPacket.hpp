#ifndef GAME_NETWORK_CLIENTBOUNDADDENTITYPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDADDENTITYPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;

class ClientboundAddEntityPacket : public Packet, public boost::enable_shared_from_this<ClientboundAddEntityPacket>
{
public:
    ClientboundAddEntityPacket();
    ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int data);
    ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int yRot, int xRot, int xa, int ya, int za);
    ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int data, int yRot, int xRot, int xa, int ya, int za);
    ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int data, const BlockPos& pos, int yRot, int xRot, int xa, int ya, int za);
    virtual ~ClientboundAddEntityPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    void _init(boost::shared_ptr<Entity> entity, int type, int data, int xa, int ya, int za, int yRot, int xRot);
    int getId();
    std::wstring getUUID();
    int getX();
    int getY();
    int getZ();
    double getXa();
    double getYa();
    double getZa();
    unsigned char getxRot();
    unsigned char getyRot();
    int getType();
    int getData();
    void setXa(int xa);
    void setYa(int ya);
    void setZa(int za);
    void setData(int data);

    static boost::shared_ptr<Packet> create();

private:
    int mId;
    std::wstring mUUID;
    int mX;
    int mY;
    int mZ;
    int mXa;
    int mYa;
    int mZa;
    int mType;
    int mData;
    unsigned char mYRot;
    unsigned char mXRot;
};

#endif
