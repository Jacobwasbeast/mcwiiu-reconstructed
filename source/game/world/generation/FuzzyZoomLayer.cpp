#include "game/world/generation/FuzzyZoomLayer.hpp"

FuzzyZoomLayer::FuzzyZoomLayer(long long seed, boost::shared_ptr<Layer> parent)
    : ZoomLayer(seed, parent)
{
}

FuzzyZoomLayer::~FuzzyZoomLayer()
{
}

int FuzzyZoomLayer::modeOrRandom(int upperLeft, int upperRight, int lowerLeft, int lowerRight)
{
    return random(upperLeft, upperRight, lowerLeft, lowerRight);
}

boost::shared_ptr<Layer> FuzzyZoomLayer::zoom(long long seed, boost::shared_ptr<Layer> parent, int count)
{
    boost::shared_ptr<Layer> layer = parent;

    for (int i = 0; i < count; i++)
    {
        layer = boost::shared_ptr<Layer>(new FuzzyZoomLayer(seed + i, layer));
    }

    return layer;
}
