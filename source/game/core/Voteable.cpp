#include "game/core/Voteable.hpp"

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/Minecraft.hpp"
#include "game/util/Random.hpp"
#include "game/network/VotePacket.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/world/generation/PlayerList.hpp"

class MinecraftServer;
extern MinecraftServer* gMinecraftServer;

Voteable::Voteable(int id, bool broadcast, bool singleVote)
    : mId(id), mBroadcast(broadcast), mSingleVote(singleVote), mVotes()
{
}

Voteable::~Voteable()
{
}

std::map<int, int> Voteable::getVotesFromPlayer(int playerId)
{
    std::map<int, int> result;
    boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it;
    for (it = mVotes.begin(); it != mVotes.end(); ++it)
    {
        if (it->first.first == playerId)
            result[it->first.second] = static_cast<int>(it->second);
    }
    return result;
}

std::map<int, int> Voteable::getVotesToVoteable(int voteableId)
{
    std::map<int, int> result;
    boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it;
    for (it = mVotes.begin(); it != mVotes.end(); ++it)
    {
        if (it->first.second == voteableId)
            result[it->first.first] = static_cast<int>(it->second);
    }
    return result;
}

boost::unordered_map<int, int> Voteable::getTotalledVotes()
{
    boost::unordered_map<int, int> result;
    boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it;
    for (it = mVotes.begin(); it != mVotes.end(); ++it)
    {
        if (it->second >= 0)
            result[it->first.second] += static_cast<int>(it->second);
    }
    return result;
}

int Voteable::getWinningVote(int& totalVotes)
{
    boost::unordered_map<int, int> totals = getTotalledVotes();
    Random random;
    int winningVote = -1;
    int winningTotal = 0;
    boost::unordered_map<int, int>::const_iterator it;
    for (it = totals.begin(); it != totals.end(); ++it)
    {
        if (it->second > 0 &&
            (winningTotal < it->second ||
             (winningTotal == it->second && random.nextBoolean())))
        {
            winningVote = it->first;
            winningTotal = it->second;
        }
    }
    totalVotes = winningTotal;
    return winningVote;
}

int Voteable::getOnlyVote(int playerId, int* vote) const
{
    boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it;
    for (it = mVotes.begin(); it != mVotes.end(); ++it)
    {
        if (it->first.first == playerId)
        {
            *vote = static_cast<int>(it->first.second);
            return 1;
        }
    }
    return 0;
}

std::pair<int, int> Voteable::getKey(int first, int second)
{
    return std::make_pair(first, second);
}

int Voteable::getVote(int playerId, int voteableId)
{
    boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it =
        mVotes.find(getKey(playerId, voteableId));
    return it == mVotes.end() ? -1 : static_cast<int>(it->second);
}

int Voteable::getVote(boost::shared_ptr<MultiplayerLocalPlayer> player, int voteableId)
{
    if (player == 0)
        return -1;
    return getVote(player->getSmallId(), voteableId);
}

void Voteable::setVote(int playerId, int voteableId, char vote)
{
    if (vote == -1)
    {
        boost::unordered_map<std::pair<int, int>, char, KeyHasher>::iterator it =
            mVotes.find(getKey(playerId, voteableId));
        if (it != mVotes.end())
            mVotes.erase(it);
        return;
    }
    if (mSingleVote)
    {
        int oldVote = -1;
        if (getOnlyVote(playerId, &oldVote))
        {
            boost::unordered_map<std::pair<int, int>, char, KeyHasher>::iterator it =
                mVotes.find(getKey(playerId, oldVote));
            if (it != mVotes.end())
                mVotes.erase(it);
        }
    }
    mVotes[getKey(playerId, voteableId)] = vote;
}

void Voteable::setVoteAndSync(int playerId, int voteableId, char vote)
{
    if (getVote(playerId, voteableId) == vote)
        return;
    setVote(playerId, voteableId, vote);
    boost::shared_ptr<Packet> packet(new VotePacket(mId, vote, playerId, voteableId));
    if (mBroadcast)
        gMinecraftServer->getPlayers()->broadcastAll(packet);
    else
        Minecraft::GetInstance()->mPlayer->mConnection->send(packet);
}

void Voteable::setIncludedVotesAndSync(int playerId, char vote)
{
    std::vector<std::pair<int, int> > included;
    boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it;
    for (it = mVotes.begin(); it != mVotes.end(); ++it)
    {
        if (it->first.first == playerId)
            included.push_back(it->first);
    }
    std::vector<std::pair<int, int> >::const_iterator selected;
    for (selected = included.begin(); selected != included.end(); ++selected)
        setVoteAndSync(selected->first, selected->second, vote);
}

void Voteable::setIncludedVotesAndSync(int playerId, int voteableId, char vote)
{
    std::vector<std::pair<int, int> > included;
    boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it;
    for (it = mVotes.begin(); it != mVotes.end(); ++it)
    {
        if (it->first.first == playerId || it->first.second == voteableId)
            included.push_back(it->first);
    }
    std::vector<std::pair<int, int> >::const_iterator selected;
    for (selected = included.begin(); selected != included.end(); ++selected)
        setVoteAndSync(selected->first, selected->second, vote);
}

void Voteable::syncAllVotesToPlayer(boost::shared_ptr<ServerPlayer> player)
{
    if (!mBroadcast || player == 0 || player->connection == 0)
        return;
    boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it;
    for (it = mVotes.begin(); it != mVotes.end(); ++it)
    {
        boost::shared_ptr<Packet> packet(
            new VotePacket(mId, it->second, it->first.first, it->first.second));
        player->connection->send(packet);
    }
}

void Voteable::clearVotes()
{
    mVotes.clear();
}
