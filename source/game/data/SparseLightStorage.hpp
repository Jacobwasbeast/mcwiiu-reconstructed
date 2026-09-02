#ifndef GAME_DATA_SPARSELIGHTSTORAGE_HPP
#define GAME_DATA_SPARSELIGHTSTORAGE_HPP

#include <vector>

#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;

class SparseLightStorage
{
public:
    SparseLightStorage(bool allBright);
    SparseLightStorage(bool allBright, bool compressed);
    SparseLightStorage(SparseLightStorage* other);
    ~SparseLightStorage();

    void setData(arrayWithLength<unsigned char> data, unsigned int offset);
    void getData(arrayWithLength<unsigned char> data, unsigned int offset);

    int get(int x, int y, int z);
    void set(int x, int y, int z, int value);
    void setAllBright();

    int setDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset);
    int getDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset);

    unsigned int compress();
    bool isCompressed();

    int getAllocatedBytes();

    unsigned int getRawData(unsigned char* buffer);
    void setFromRawData(unsigned char* rawData, unsigned int size);

    void write(DataOutputStream* out);
    void read(DataInputStream* in, unsigned char* buffer);

    static void staticCtor();
    static unsigned int getRawDataMaxBytes();
    static void tick();

private:
    int getDataSize(long long countAndData);
    void queueForDelete(unsigned char* data, int size);
    void updateDataAndCount(long long countAndData);
    void getPlaneIndicesAndData(unsigned char** planeIndices, unsigned char** planeData);
    void addNewPlane(std::vector<int>& planes);

    long long mPlaneCountAndData;
};

#endif
