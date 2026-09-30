#include "game/core/Voteable.hpp"

#include <map>
#include <vector>

#include <boost/make_shared.hpp>

#include "game/core/Minecraft.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/VotePacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

std::size_t Voteable::KeyHasher::operator()(const std::pair<int, int>& key) const
{
    const std::size_t first = static_cast<std::size_t>(static_cast<unsigned int>(key.first));
    const std::size_t second = static_cast<std::size_t>(static_cast<unsigned int>(key.second));
    return first ^ (second + static_cast<std::size_t>(0x9e3779b9u) + (first << 6) + (first >> 2));
}

Voteable::Voteable(int voteableId, bool isServer, bool oneVotePerPlayer)
    : mVoteableId(voteableId), mIsServer(isServer), mOneVotePerPlayer(oneVotePerPlayer), mVotes()
{
}

Voteable::~Voteable() {}

std::map<int, int> Voteable::getVotesFromPlayer(int playerId) const
{
    std::map<int, int> votes;
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        if (it->first.first == playerId) votes[it->first.second] = static_cast<int>(it->second);
    }
    return votes;
}

std::map<int, int> Voteable::getVotesToVoteable(int voteableId) const
{
    std::map<int, int> votes;
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        if (it->first.second == voteableId) votes[it->first.first] = static_cast<int>(it->second);
    }
    return votes;
}

boost::unordered_map<int, int> Voteable::getTotalledVotes() const
{
    boost::unordered_map<int, int> totals;
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        if (it->second >= 0) totals[it->first.second] += static_cast<int>(it->second);
    }
    return totals;
}

int Voteable::getWinningVote(int& voteCount) const
{
    const boost::unordered_map<int, int> totals = getTotalledVotes();
    Random random;
    int winningVote = -1;
    voteCount = 0;
    for (boost::unordered_map<int, int>::const_iterator it = totals.begin(); it != totals.end(); ++it) {
        if (it->second > 0 && (it->second > voteCount || (it->second == voteCount && random.nextBoolean()))) {
            winningVote = it->first;
            voteCount = it->second;
        }
    }
    if (winningVote < 0) voteCount = 0;
    return winningVote;
}

bool Voteable::getOnlyVote(int playerId, int* voteableId) const
{
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        if (it->first.first == playerId) {
            *voteableId = it->first.second;
            return true;
        }
    }
    return false;
}

std::pair<int, int> Voteable::getKey(int playerId, int voteableId)
{
    return std::make_pair(playerId, voteableId);
}

int Voteable::getVote(int playerId, int voteableId) const
{
    const boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it = mVotes.find(getKey(playerId, voteableId));
    return it == mVotes.end() ? -1 : static_cast<int>(it->second);
}

int Voteable::getVote(const boost::shared_ptr<MultiplayerLocalPlayer>& player, int voteableId) const
{
    return player ? getVote(player->mPlayerIndex, voteableId) : -1;
}

void Voteable::setVote(int playerId, int voteableId, char vote)
{
    const std::pair<int, int> key = getKey(playerId, voteableId);
    if (vote == -1) {
        mVotes.erase(key);
        return;
    }
    if (mOneVotePerPlayer) {
        int previousVoteable = 0;
        if (getOnlyVote(playerId, &previousVoteable) && previousVoteable != voteableId) mVotes.erase(getKey(playerId, previousVoteable));
    }
    mVotes[key] = vote;
}

void Voteable::setVoteAndSync(int playerId, int voteableId, char vote)
{
    const int previousVote = getVote(playerId, voteableId);
    if (previousVote == static_cast<int>(vote)) return;
    setVote(playerId, voteableId, vote);
    boost::shared_ptr<VotePacket> packet = boost::make_shared<VotePacket>(mVoteableId, vote, playerId, voteableId);
    if (mIsServer) {
        gMinecraftServer->getPlayers()->broadcastAll(packet);
    } else {
        Minecraft::GetInstance()->mPlayer->mPacketListener->send(packet);
    }
}

void Voteable::setIncludedVotesAndSync(int playerId, char vote)
{
    std::vector<std::pair<int, int> > keys;
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        if (it->first.first == playerId) keys.push_back(it->first);
    }
    for (std::vector<std::pair<int, int> >::const_iterator it = keys.begin(); it != keys.end(); ++it) setVoteAndSync(it->first, it->second, vote);
}

void Voteable::setIncludedVotesAndSync(int playerId, int voteableId, char vote)
{
    std::vector<std::pair<int, int> > keys;
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        if (it->first.first == playerId || it->first.second == voteableId) keys.push_back(it->first);
    }
    for (std::vector<std::pair<int, int> >::const_iterator it = keys.begin(); it != keys.end(); ++it) setVoteAndSync(it->first, it->second, vote);
}

void Voteable::syncAllVotesToPlayer(const boost::shared_ptr<ServerPlayer>& player) const
{
    if (!mIsServer) return;
    for (boost::unordered_map<std::pair<int, int>, char, KeyHasher>::const_iterator it = mVotes.begin(); it != mVotes.end(); ++it) {
        player->mConnection->send(boost::make_shared<VotePacket>(mVoteableId, it->second, it->first.first, it->first.second));
    }
}

void Voteable::clearVotes() { mVotes.clear(); }
