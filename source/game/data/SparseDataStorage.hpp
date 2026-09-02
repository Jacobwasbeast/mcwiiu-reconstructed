#ifndef GAME_DATA_SPARSEDATASTORAGE_HPP
#define GAME_DATA_SPARSEDATASTORAGE_HPP

#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;

class SparseDataStorage
{
public:
    class Array256_1Bit
    {
    public:
        void init();

        bool set(unsigned char index, unsigned char value);
        unsigned char get(unsigned char index);

    private:
        unsigned char mBits[32];
        char mValues[4];
    };

    class Array256_2Bit
    {
    public:
        void init();

        bool set(unsigned char index, unsigned char value);
        unsigned char get(unsigned char index);

    private:
        unsigned char mBits[64];
        char mValues[4];
    };

    class PlaneOffsets
    {
    public:
        void initPlane(int plane, void* planeData);

        void setDataOffset(int plane, unsigned short offset);
        unsigned short getDataOffset(int plane);

        bool setData(int plane, int index, int value);
        int getData(int plane, int index);
        void getData(int plane, char (&values)[256]);

        unsigned char getDataSizeFromType(int type);
        unsigned char getDataSize(int plane);

        int getValuesInPlane(int plane, char (&values)[16]);

        unsigned short mOffsets[128];
    };

    SparseDataStorage();
    SparseDataStorage(bool compressed);
    SparseDataStorage(SparseDataStorage* other);
    ~SparseDataStorage();

    bool isSameAs(SparseDataStorage* other);

    int get(int x, int y, int z);
    void set(int x, int y, int z, int value);

    void setData(arrayWithLength<unsigned char> data, unsigned int offset, bool compressed);
    void getData(arrayWithLength<unsigned char> data, unsigned int offset, int y0, int y1);
    int getValuesInPlane(arrayWithLength<unsigned char> data, unsigned int offset, int plane, char (&values)[16]);

    int setDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset, void (*callback)(int, int, int, void*, int), void* callbackData, int callbackValue);
    int testSetDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset, bool* changed);
    int getDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset);

    void increasePlaneBitdepth(int plane);

    unsigned int compress();
    bool isCompressed();

    void write(DataOutputStream* out);
    void read(DataInputStream* in);

    static void staticCtor();
    static void tick();

private:
    PlaneOffsets* allocatePlaneOffsets(unsigned int dataSize);
    static void freePlaneOffsets(PlaneOffsets* planeOffsets, int dataSize);

    void queueForDelete(unsigned char* data, int dataSize);

    long long createDataAndCount(PlaneOffsets* planeOffsets, int dataSize);
    void updateDataAndCount(PlaneOffsets* planeOffsets, int dataSize);
    bool tryUpdateDataAndCount(PlaneOffsets* planeOffsets, int dataSize);

    PlaneOffsets* getPlaneOffsets();
    unsigned int getPlaneOffsetsDataSize();

    long long mDataAndCount;
};

#endif
