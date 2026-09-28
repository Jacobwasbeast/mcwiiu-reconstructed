#include "game/core/VoronoiZoom.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/System.hpp"

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
    x -= 2;
    z -= 2;
    int parentX = x >> 2;
    int parentZ = z >> 2;
    int parentWidth = (width >> 2) + 2;
    int parentHeight = (height >> 2) + 2;
    arrayWithLength<int> parentArea = parent->getArea(parentX, parentZ, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "VoronoiZoom::getArea");

    int gridWidth = (parentWidth - 1) << 2;
    int gridHeight = (parentHeight - 1) << 2;
    arrayWithLength<int> grid = IntCache::allocate(gridWidth * gridHeight);

    for (int row = 0; row < parentHeight - 1; row++)
    {
        int upperLeft = parentArea.data[row * parentWidth];
        int lowerLeft = parentArea.data[(row + 1) * parentWidth];

        for (int column = 0; column < parentWidth - 1; column++)
        {
            initRandom((long long)((column + parentX) << 2), (long long)((row + parentZ) << 2));
            double upperLeftX = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6;
            double upperLeftZ = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6;
            initRandom((long long)((column + parentX + 1) << 2), (long long)((row + parentZ) << 2));
            double upperRightX = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6 + 4.0;
            double upperRightZ = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6;
            initRandom((long long)((column + parentX) << 2), (long long)((row + parentZ + 1) << 2));
            double lowerLeftX = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6;
            double lowerLeftZ = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6 + 4.0;
            initRandom((long long)((column + parentX + 1) << 2), (long long)((row + parentZ + 1) << 2));
            double lowerRightX = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6 + 4.0;
            double lowerRightZ = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6 + 4.0;

            int upperRight = parentArea.data[column + 1 + row * parentWidth] & 0xff;
            int lowerRight = parentArea.data[column + 1 + (row + 1) * parentWidth] & 0xff;

            for (int localZ = 0; localZ < 4; localZ++)
            {
                int index = ((row << 2) + localZ) * gridWidth + (column << 2);

                for (int localX = 0; localX < 4; localX++)
                {
                    double upperLeftDistance = ((double)localZ - upperLeftZ) * ((double)localZ - upperLeftZ)
                        + ((double)localX - upperLeftX) * ((double)localX - upperLeftX);
                    double upperRightDistance = ((double)localZ - upperRightZ) * ((double)localZ - upperRightZ)
                        + ((double)localX - upperRightX) * ((double)localX - upperRightX);
                    double lowerLeftDistance = ((double)localZ - lowerLeftZ) * ((double)localZ - lowerLeftZ)
                        + ((double)localX - lowerLeftX) * ((double)localX - lowerLeftX);
                    double lowerRightDistance = ((double)localZ - lowerRightZ) * ((double)localZ - lowerRightZ)
                        + ((double)localX - lowerRightX) * ((double)localX - lowerRightX);

                    if (upperLeftDistance < upperRightDistance && upperLeftDistance < lowerLeftDistance
                        && upperLeftDistance < lowerRightDistance)
                    {
                        grid.data[index++] = upperLeft;
                    }
                    else if (upperRightDistance < upperLeftDistance && upperRightDistance < lowerLeftDistance
                             && upperRightDistance < lowerRightDistance)
                    {
                        grid.data[index++] = upperRight;
                    }
                    else if (lowerLeftDistance < upperLeftDistance && lowerLeftDistance < upperRightDistance
                             && lowerLeftDistance < lowerRightDistance)
                    {
                        grid.data[index++] = lowerLeft;
                    }
                    else
                    {
                        grid.data[index++] = lowerRight;
                    }
                }
            }

            upperLeft = upperRight;
            lowerLeft = lowerRight;
        }
    }

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int row = 0; row < height; row++)
    {
        System::arraycopy(grid, (unsigned int)((row + (z & 3)) * gridWidth + (x & 3)),
                          &area, (unsigned int)(row * width), (unsigned int)width);
    }

    PIXEndNamedEvent();

    return area;
}
