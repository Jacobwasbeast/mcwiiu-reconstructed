#ifndef GAME_CORE_VOTEABLES_HPP
#define GAME_CORE_VOTEABLES_HPP

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

class Voteable;
class ServerPlayer;

class Voteables : public boost::unordered_map<int, Voteable*>
{
public:
    Voteables();
    virtual ~Voteables();
    Voteable* getVoteable(unsigned int id);
    void init();
    void addVoteable(Voteable* voteable);
    void removeVoteable(int id);
    void syncAllVoteablesToPlayer(boost::shared_ptr<ServerPlayer> player);
};

#endif
