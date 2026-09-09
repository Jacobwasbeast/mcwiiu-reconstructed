#include "game/core/Voteables.hpp"
#include "game/core/Voteable.hpp"

#include <boost/shared_ptr.hpp>

Voteables::Voteables()
    : boost::unordered_map<int, Voteable*>()
{
    init();
}

Voteables::~Voteables()
{
    clear();
}

Voteable* Voteables::getVoteable(unsigned int id)
{
    iterator found = find(static_cast<int>(id));
    if (found == end())
        return 0;
    return found->second;
}

void Voteables::init()
{
    clear();
}

void Voteables::addVoteable(Voteable* voteable)
{
    if (voteable == 0)
        return;
    const int id = *reinterpret_cast<const int*>(voteable);
    if (find(id) == end())
        emplace(id, voteable);
}

void Voteables::removeVoteable(int id)
{
    iterator found = find(id);
    if (found != end())
        erase(found);
}

void Voteables::syncAllVoteablesToPlayer(boost::shared_ptr<ServerPlayer> player)
{
    if (empty())
        return;

    for (iterator current = begin(); current != end(); ++current)
    {
        if (current->second != 0)
            current->second->syncAllVotesToPlayer(player);
    }
}
