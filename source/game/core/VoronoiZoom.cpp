#include "game/core/VoronoiZoom.hpp"

#include "game/core/IntCache.hpp"

#include <algorithm>

VoronoiZoom::VoronoiZoom(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

VoronoiZoom::~VoronoiZoom()
{
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

    int zoomedWidth = (parentWidth - 1) * 4;

    arrayWithLength<int> zoomedArea = IntCache::allocate(zoomedWidth * ((parentHeight - 1) * 4));

    for (int i = 0; i < parentHeight - 1; i++)
    {
        int rowIndex = i * zoomedWidth * 4;

        int upperLeft = parentArea.data[i * parentWidth];
        int lowerLeft = parentArea.data[(i + 1) * parentWidth];

        for (int j = 0; j < parentWidth - 1; j++)
        {
            initRandom((long long)((j + parentX) * 4), (long long)((i + parentZ) * 4));
            double upperLeftX = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6;
            double upperLeftZ = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6;

            initRandom((long long)((j + parentX + 1) * 4), (long long)((i + parentZ) * 4));
            double upperRightX = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6 + 4.0;
            double upperRightZ = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6;

            initRandom((long long)((j + parentX) * 4), (long long)((i + parentZ + 1) * 4));
            double lowerLeftX = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6;
            double lowerLeftZ = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6 + 4.0;

            initRandom((long long)((j + parentX + 1) * 4), (long long)((i + parentZ + 1) * 4));
            double lowerRightX = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6 + 4.0;
            double lowerRightZ = ((double)nextRandom(1024) / 1024.0 - 0.5) * 3.6 + 4.0;

            int upperRight = parentArea.data[i * parentWidth + j + 1] & 255;
            int lowerRight = parentArea.data[(i + 1) * parentWidth + j + 1] & 255;

            for (int localZ = 0; localZ < 4; localZ++)
            {
                int destIndex = rowIndex + localZ * zoomedWidth + j * 4;

                double upperLeftDeltaZ = (double)localZ - upperLeftZ;
                double upperRightDeltaZ = (double)localZ - upperRightZ;
                double lowerLeftDeltaZ = (double)localZ - lowerLeftZ;
                double lowerRightDeltaZ = (double)localZ - lowerRightZ;

                for (int localX = 0; localX < 4; localX++)
                {
                    double upperLeftDistance = upperLeftDeltaZ * upperLeftDeltaZ +
                                               ((double)localX - upperLeftX) * ((double)localX - upperLeftX);
                    double upperRightDistance = upperRightDeltaZ * upperRightDeltaZ +
                                                ((double)localX - upperRightX) * ((double)localX - upperRightX);
                    double lowerLeftDistance = lowerLeftDeltaZ * lowerLeftDeltaZ +
                                               ((double)localX - lowerLeftX) * ((double)localX - lowerLeftX);
                    double lowerRightDistance = lowerRightDeltaZ * lowerRightDeltaZ +
                                                ((double)localX - lowerRightX) * ((double)localX - lowerRightX);

                    if (upperLeftDistance < upperRightDistance &&
                        upperLeftDistance < lowerLeftDistance &&
                        upperLeftDistance < lowerRightDistance)
                    {
                        zoomedArea.data[destIndex] = upperLeft;
                    }
                    else if (upperRightDistance < upperLeftDistance &&
                             upperRightDistance < lowerLeftDistance &&
                             upperRightDistance < lowerRightDistance)
                    {
                        zoomedArea.data[destIndex] = upperRight;
                    }
                    else if (lowerLeftDistance < upperLeftDistance &&
                             lowerLeftDistance < upperRightDistance &&
                             lowerLeftDistance < lowerRightDistance)
                    {
                        zoomedArea.data[destIndex] = lowerLeft;
                    }
                    else
                    {
                        zoomedArea.data[destIndex] = lowerRight;
                    }

                    destIndex++;
                }
            }

            upperLeft = upperRight;
            lowerLeft = lowerRight;
        }
    }

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        std::copy(zoomedArea.data + (i + (z & 3)) * zoomedWidth + (x & 3),
                  zoomedArea.data + (i + (z & 3)) * zoomedWidth + (x & 3) + width,
                  area.data + i * width);
    }

    return area;
}
