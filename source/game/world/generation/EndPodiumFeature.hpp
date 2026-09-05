#ifndef GAME_WORLD_GENERATION_ENDPODIUMFEATURE_HPP
#define GAME_WORLD_GENERATION_ENDPODIUMFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class EndPodiumFeature : public Feature
{
public:
    EndPodiumFeature(bool active);
    virtual ~EndPodiumFeature();

    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    bool mActive;
};

#endif
