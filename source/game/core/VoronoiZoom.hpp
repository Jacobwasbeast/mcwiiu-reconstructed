#ifndef GAME_CORE_VORONOIZOOM_HPP
#define GAME_CORE_VORONOIZOOM_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class VoronoiZoom : public Layer
{
public:
    VoronoiZoom(long long seed, boost::shared_ptr<Layer> parent);
    virtual ~VoronoiZoom();

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);
};

#endif
