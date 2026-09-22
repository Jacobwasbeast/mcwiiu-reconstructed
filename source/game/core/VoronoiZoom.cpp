#include "game/core/VoronoiZoom.hpp"
#include "game/core/System.hpp"
#include "game/core/IntCache.hpp"

#include <boost/assert.hpp>

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();
extern void instructionSynchronize();

VoronoiZoom::VoronoiZoom(long long seed, boost::shared_ptr<Layer> parentLayer)
    : Layer(seed)
{
    parent = parentLayer;
}

VoronoiZoom::~VoronoiZoom()
{
    parent.reset();
}

arrayWithLength<int> VoronoiZoom::getArea(int x, int z, int width, int height)
{
    const int parentX = (x - 2) >> 2;
    const int parentZ = (z - 2) >> 2;
    const int parentWidth = (width >> 2) + 2;
    const int parentHeight = (height >> 2) + 2;
    arrayWithLength<int> source = parent->getArea(parentX, parentZ, parentWidth, parentHeight);
    const int gridWidth = parentWidth * 4 - 4;
    const int gridHeight = parentHeight * 4 - 4;
    PIXBeginNamedEvent(1.65f, "VoronoiZoom::getArea");
    arrayWithLength<int> grid = IntCache::allocate(gridWidth * gridHeight);
    for (int row = 0; row < parentHeight - 1; ++row)
    {
        for (int column = 0; column < parentWidth - 1; ++column)
        {
            const int upperLeft = source.data[row * parentWidth + column];
            const int upperRight = source.data[row * parentWidth + column + 1];
            const int lowerLeft = source.data[(row + 1) * parentWidth + column];
            const int lowerRight = source.data[(row + 1) * parentWidth + column + 1];
            initRandom((parentX + column) * 4, (parentZ + row) * 4);
            const double x0 = static_cast<double>(nextRandom(1024)) * 3.6 / 1024.0 - 1.8;
            const double z0 = static_cast<double>(nextRandom(1024)) * 3.6 / 1024.0 - 1.8;
            initRandom((parentX + column + 1) * 4, (parentZ + row) * 4);
            const double x1 = static_cast<double>(nextRandom(1024)) * 3.6 / 1024.0 - 1.8 + 4.0;
            const double z1 = static_cast<double>(nextRandom(1024)) * 3.6 / 1024.0 - 1.8;
            initRandom((parentX + column) * 4, (parentZ + row + 1) * 4);
            const double x2 = static_cast<double>(nextRandom(1024)) * 3.6 / 1024.0 - 1.8;
            const double z2 = static_cast<double>(nextRandom(1024)) * 3.6 / 1024.0 - 1.8 + 4.0;
            initRandom((parentX + column + 1) * 4, (parentZ + row + 1) * 4);
            const double x3 = static_cast<double>(nextRandom(1024)) * 3.6 / 1024.0 - 1.8 + 4.0;
            const double z3 = static_cast<double>(nextRandom(1024)) * 3.6 / 1024.0 - 1.8 + 4.0;
            for (int localZ = 0; localZ < 4; ++localZ)
            {
                for (int localX = 0; localX < 4; ++localX)
                {
                    const double px = static_cast<double>(localX);
                    const double pz = static_cast<double>(localZ);
                    const double d0 = (px - x0) * (px - x0) + (pz - z0) * (pz - z0);
                    const double d1 = (px - x1) * (px - x1) + (pz - z1) * (pz - z1);
                    const double d2 = (px - x2) * (px - x2) + (pz - z2) * (pz - z2);
                    const double d3 = (px - x3) * (px - x3) + (pz - z3) * (pz - z3);
                    int value;
                    if (d0 < d1 && d0 < d2 && d0 < d3)
                        value = upperLeft;
                    else if (d1 < d0 && d1 < d2 && d1 < d3)
                        value = upperRight;
                    else if (d2 < d0 && d2 < d1 && d2 < d3)
                        value = lowerLeft;
                    else
                        value = lowerRight;
                    grid.data[(row * 4 + localZ) * gridWidth + column * 4 + localX] = value;
                }
            }
        }
    }
    arrayWithLength<int> result = IntCache::allocate(width * height);
    const int sourceOffset = ((z - 2) & 3) * gridWidth + ((x - 2) & 3);
    for (int row = 0; row < height; ++row)
        System::arraycopy(grid, static_cast<unsigned int>(sourceOffset + row * gridWidth),
                          &result, static_cast<unsigned int>(row * width),
                          static_cast<unsigned int>(width));
    PIXEndNamedEvent();
    instructionSynchronize();
    return result;
}
