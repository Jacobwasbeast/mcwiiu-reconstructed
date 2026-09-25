#include "game/world/generation/SmoothZoomLayer.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

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
    int parentWidth = (width >> 1) + 3;
    int parentHeight = (height >> 1) + 3;
    arrayWithLength<int> parentArea = parent->getArea(parentX, parentZ, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "SmoothZoomLayer::getArea");

    int zoomedWidth = parentWidth * 2;
    arrayWithLength<int> zoomedArea = IntCache::allocate(zoomedWidth * (parentHeight * 2));
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
            zoomedArea.data[dest + zoomedWidth] = upperLeft + (lowerLeft - upperLeft) * nextRandom(256) / 256;
            zoomedArea.data[dest + 1] = upperLeft + (upperRight - upperLeft) * nextRandom(256) / 256;
            int upper = upperLeft + (upperRight - upperLeft) * nextRandom(256) / 256;
            int lower = lowerLeft + (lowerRight - lowerLeft) * nextRandom(256) / 256;
            zoomedArea.data[dest + 1 + zoomedWidth] = upper + (lower - upper) * nextRandom(256) / 256;

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

    PIXEndNamedEvent();

    return area;
}
