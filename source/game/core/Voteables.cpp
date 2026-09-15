#include "game/core/Voteables.hpp"

#include "game/core/Voteable.hpp"

Voteables::Voteables()
    : boost::unordered_map<int, Voteable*>(11)
{
}

Voteables::~Voteables()
{
    for (boost::unordered_map<int, Voteable*>::iterator it = begin(); it != end(); ++it)
    {
        delete it->second;
    }
    clear();
}

Voteable* Voteables::getVoteable(unsigned int voteableId)
{
    boost::unordered_map<int, Voteable*>::iterator it = find(static_cast<int>(voteableId));
    return it == end() ? NULL : it->second;
}

void Voteables::init()
{
}

void Voteables::addVoteable(Voteable* voteable)
{
    const int voteableId = voteable->mVoteableId;
    if (find(voteableId) == end())
    {
        (*this)[voteableId] = voteable;
    }
}

uint64_t Voteables::syncAllVoteablesToPlayer(boost::shared_ptr<ServerPlayer> player)
{
    for (boost::unordered_map<int, Voteable*>::iterator it = begin(); it != end(); ++it)
    {
        it->second->syncAllVotesToPlayer(player);
    }
    return 0;
}
