#ifndef GAME_CORE_VOTEABLES_HPP
#define GAME_CORE_VOTEABLES_HPP

#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

class ServerPlayer;
class Voteable;

class Voteables : public boost::unordered_map<int, Voteable*>
{
public:
    Voteables();
    virtual ~Voteables();
    Voteable* getVoteable(unsigned int voteableId);
    void init();
    void addVoteable(Voteable* voteable);
    uint64_t syncAllVoteablesToPlayer(boost::shared_ptr<ServerPlayer> player);
    void removeVoteable(int voteableId);
};

#endif
