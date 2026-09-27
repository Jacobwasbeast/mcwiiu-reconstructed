#ifndef GAME_NETWORK_CHUNKHISTORY_HPP
#define GAME_NETWORK_CHUNKHISTORY_HPP

#include <vector>

class ChunkHistory
{
public:
    virtual std::vector<short>* GetChangesForChunk(int x, int z) const = 0;
    virtual bool ShouldLoadOnClient(int x, int z) const = 0;
};

#endif
