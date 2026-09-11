#include "game/network/ClientboundSetPassengersPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetPassengersPacket::ClientboundSetPassengersPacket()
    : mVehicle(0), mPassengers(NULL), mPassengerCount(0)
{
}

ClientboundSetPassengersPacket::ClientboundSetPassengersPacket(boost::shared_ptr<Entity> vehicle)
    : mVehicle(0), mPassengers(NULL), mPassengerCount(0)
{
    if (vehicle)
    {
        mVehicle = vehicle->getId();
        std::vector<boost::weak_ptr<Entity> >* passengers = vehicle->getPassengers();
        if (passengers != NULL)
        {
            std::vector<int> ids;
            for (std::vector<boost::weak_ptr<Entity> >::iterator it = passengers->begin(); it != passengers->end(); ++it)
            {
                boost::shared_ptr<Entity> passenger = it->lock();
                if (passenger)
                {
                    ids.push_back(passenger->getId());
                }
            }
            mPassengerCount = ids.size();
            if (mPassengerCount != 0)
            {
                mPassengers = new int[mPassengerCount];
                for (unsigned int i = 0; i < mPassengerCount; ++i)
                    mPassengers[i] = ids[i];
            }
        }
    }
}

ClientboundSetPassengersPacket::~ClientboundSetPassengersPacket()
{
    delete[] mPassengers;
}

int ClientboundSetPassengersPacket::getPacketId()
{
    return 134;
}

void ClientboundSetPassengersPacket::read(DataInputStream* input)
{
    mVehicle = input->readVarInt();
    arrayWithLength<int> values = input->readVarIntArray();
    delete[] mPassengers;
    mPassengers = values.data;
    mPassengerCount = values.length;
}

void ClientboundSetPassengersPacket::write(DataOutputStream* output)
{
    output->writeVarInt(mVehicle);
    arrayWithLength<int> values;
    values.data = mPassengers;
    values.length = mPassengerCount;
    output->writeVarIntArray(values);
}

void ClientboundSetPassengersPacket::handle(PacketListener* listener)
{
    listener->handleSetEntityPassengersPacket(shared_from_this());
}

arrayWithLength<int> ClientboundSetPassengersPacket::getPassengers()
{
    return *reinterpret_cast<arrayWithLength<int>*>(&mPassengers);
}

int ClientboundSetPassengersPacket::getVehicle()
{
    return mVehicle;
}

boost::shared_ptr<Packet> ClientboundSetPassengersPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundSetPassengersPacket());
}
