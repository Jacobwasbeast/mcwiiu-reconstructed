#include "game/world/generation/SmoothZoomLayer.hpp"
#include "game/core/IntCache.hpp"

SmoothZoomLayer::SmoothZoomLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

SmoothZoomLayer::~SmoothZoomLayer()
{
}

boost::shared_ptr<Layer> SmoothZoomLayer::zoom(long long seed,
    boost::shared_ptr<Layer> parent, int count)
{
    boost::shared_ptr<Layer> layer = parent;
    for (int i = 0; i < count; ++i)
    {
        layer = boost::shared_ptr<Layer>(new SmoothZoomLayer(seed + i, layer));
    }
    return layer;
}

arrayWithLength<int> SmoothZoomLayer::getArea(int x, int z, int width, int height)
{
    int parentX = x >> 1;
    int parentZ = z >> 1;
    int parentWidth = (width >> 1) + 2;
    int parentHeight = (height >> 1) + 2;
    arrayWithLength<int> parentArea = parent->getArea(parentX, parentZ, parentWidth, parentHeight);
    int zoomedWidth = (parentWidth - 1) * 2;
    arrayWithLength<int> zoomedArea = IntCache::allocate(zoomedWidth * ((parentHeight - 1) * 2));
    for (int i = 0; i < parentHeight - 1; ++i)
    {
        int dest = i * zoomedWidth * 2;
        int upperLeft = parentArea.data[i * parentWidth];
        int lowerLeft = parentArea.data[(i + 1) * parentWidth];
        for (int j = 0; j < parentWidth - 1; ++j)
        {
            initRandom((long long)((j + parentX) * 2), (long long)((i + parentZ) * 2));
            int upperRight = parentArea.data[i * parentWidth + j + 1];
            int lowerRight = parentArea.data[(i + 1) * parentWidth + j + 1];
            zoomedArea.data[dest] = upperLeft;
            zoomedArea.data[dest + zoomedWidth] = random(upperLeft, lowerLeft);
            zoomedArea.data[dest + 1] = random(upperLeft, upperRight);
            zoomedArea.data[dest + 1 + zoomedWidth] = modeOrRandom(upperLeft, upperRight, lowerLeft, lowerRight);
            dest += 2;
            upperLeft = upperRight;
            lowerLeft = lowerRight;
        }
    }
    arrayWithLength<int> area = IntCache::allocate(width * height);
    for (int i = 0; i < height; ++i)
    {
        int source = (i + (z & 1)) * zoomedWidth + (x & 1);
        for (int j = 0; j < width; ++j)
        {
            area.data[i * width + j] = zoomedArea.data[source + j];
        }
    }
    return area;
}
