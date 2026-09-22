#ifndef GAME_CORE_VOTEABLE_HPP
#define GAME_CORE_VOTEABLE_HPP

#include <cstdint>
#include <map>
#include <utility>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

class MultiplayerLocalPlayer;
class ServerPlayer;

class Voteable
{
public:
    struct KeyHasher
    {
        std::size_t operator()(const std::pair<int, int>& key) const;
    };

    Voteable(int voteableId, bool allowMultipleVotes, bool included);
    ~Voteable();
    std::map<int, int> getVotesFromPlayer(int playerId) const;
    std::map<int, int> getVotesToVoteable(int voteableId) const;
    boost::unordered_map<int, int> getTotalledVotes() const;
    int getWinningVote(int& voteCount) const;
    bool getOnlyVote(int playerId, int* voteableId) const;
    static std::pair<int, int> getKey(int playerId, int voteableId);
    int getVote(int playerId, int voteableId) const;
    int getVote(const boost::shared_ptr<MultiplayerLocalPlayer>& player, int voteableId) const;
    void setVote(int playerId, int voteableId, char vote);
    void setVoteAndSync(int playerId, int voteableId, char vote);
    void setIncludedVotesAndSync(int playerId, char vote);
    void setIncludedVotesAndSync(int playerId, int voteableId, char vote);
    void syncAllVotesToPlayer(const boost::shared_ptr<ServerPlayer>& player) const;
    void clearVotes();
    void setVote(int, char);
    void setVoteAndSync(int, char);

private:
    friend class Voteables;
    int mVoteableId;
    bool mAllowMultipleVotes;
    bool mIncluded;
    boost::unordered_map<std::pair<int, int>, char, KeyHasher> mVotes;
};

#endif
