#ifndef GAME_CORE_VOTEABLE_HPP
#define GAME_CORE_VOTEABLE_HPP

#include <cstddef>
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
        std::size_t operator()(const std::pair<int, int>& key) const
        {
            return static_cast<std::size_t>(key.first) ^
                   (static_cast<std::size_t>(key.second) + 0x9e3779b9u +
                    (static_cast<std::size_t>(key.first) << 6) +
                    (static_cast<std::size_t>(key.first) >> 2));
        }
    };

    Voteable(int id, bool broadcast, bool singleVote);
    ~Voteable();

    std::map<int, int> getVotesFromPlayer(int playerId);
    std::map<int, int> getVotesToVoteable(int voteableId);
    boost::unordered_map<int, int> getTotalledVotes();
    int getWinningVote(int& totalVotes);
    int getOnlyVote(int playerId, int* vote) const;
    static std::pair<int, int> getKey(int first, int second);
    int getVote(int playerId, int voteableId);
    int getVote(boost::shared_ptr<MultiplayerLocalPlayer> player, int voteableId);
    void setVote(int playerId, int voteableId, char vote);
    void setVote(int playerId, char vote);
    void setVoteAndSync(int playerId, int voteableId, char vote);
    void setVoteAndSync(int playerId, char vote);
    void setIncludedVotesAndSync(int playerId, char vote);
    void setIncludedVotesAndSync(int playerId, int voteableId, char vote);
    void syncAllVotesToPlayer(boost::shared_ptr<ServerPlayer> player);
    void clearVotes();

private:
    int mId;
    bool mBroadcast;
    bool mSingleVote;
    boost::unordered_map<std::pair<int, int>, char, KeyHasher> mVotes;
};

#endif
