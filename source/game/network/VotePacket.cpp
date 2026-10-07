#include "game/network/VotePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

VotePacket::VotePacket()
    : voteableId(-1), vote(0xff), playerId(-1), voteId(-1)
{
}

VotePacket::VotePacket(int voteableId, char vote, int playerId, int voteId)
    : voteableId(voteableId), vote(vote), playerId(playerId), voteId(voteId)
{
}

void VotePacket::read(DataInputStream* input)
{
    voteableId = input->readInt();
    vote = (char)input->readByte();
    playerId = input->readInt();
    voteId = input->readInt();
}

void VotePacket::write(DataOutputStream* output)
{
    output->writeInt(voteableId);
    output->writeByte((unsigned char)vote);
    output->writeInt(playerId);
    output->writeInt(voteId);
}

void VotePacket::handle(PacketListener* listener)
{
    listener->handleVote(shared_from_this());
}

int VotePacket::getEstimatedSize() { return 6; }
int VotePacket::getPacketId() { return 169; }

boost::shared_ptr<Packet> VotePacket::create()
{
    return boost::shared_ptr<Packet>(new VotePacket());
}
