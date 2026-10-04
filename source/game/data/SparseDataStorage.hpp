#ifndef GAME_DATA_SPARSEDATASTORAGE_HPP
#define GAME_DATA_SPARSEDATASTORAGE_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include <stdint.h>

class DataInputStream;
class DataOutputStream;

class SparseDataStorage
{
public:
    class Array256_1Bit
    {
    public:
        unsigned char indices[32];
        char palette[4];
        void init();
        unsigned int set(unsigned char index, unsigned char value);
        unsigned char get(unsigned char index);
    };

    class Array256_2Bit
    {
    public:
        unsigned char indices[64];
        char palette[4];
        void init();
        unsigned int set(unsigned char index, unsigned char value);
        unsigned char get(unsigned char index);
    };

    class PlaneOffsets
    {
    public:
        unsigned short offsets[128];
        unsigned char payload[0x3f80];
        void initPlane(int plane, void* data);
        unsigned int setData(int plane, int index, int value);
        unsigned char getDataSizeFromType(int type);
        unsigned char getDataSize(int plane);
        unsigned int getData(int plane, int index);
        unsigned int getValuesInPlane(int plane, char (&values)[16]);
        void setDataOffset(int plane, unsigned short offset);
        unsigned short getDataOffset(int plane);
        unsigned int getData(int plane, char (&values)[256]);
    };

    SparseDataStorage();
    SparseDataStorage(bool compressed);
    SparseDataStorage(SparseDataStorage* other);
    ~SparseDataStorage();
    PlaneOffsets* allocatePlaneOffsets(unsigned int size);
    static void freePlaneOffsets(PlaneOffsets* offsets, int size);
    long long createDataAndCount(PlaneOffsets* offsets, int size);
    PlaneOffsets* getPlaneOffsets();
    unsigned int getPlaneOffsetsDataSize();
    void queueForDelete(unsigned char* data, int size);
    void updateDataAndCount(PlaneOffsets* offsets, int size);
    unsigned int get(int x, int y, int z);
    unsigned int tryUpdateDataAndCount(PlaneOffsets* offsets, int size);
    static void staticCtor();
    unsigned int compress();
    static void tick();
    void write(DataOutputStream* stream);
    void read(DataInputStream* stream);
    bool isSameAs(SparseDataStorage* other);
    unsigned int getValuesInPlane(arrayWithLength<unsigned char> values, unsigned int plane, int count, char (&unique)[16]);
    void setData(arrayWithLength<unsigned char> values, unsigned int plane, bool compressed);
    void getData(arrayWithLength<unsigned char> values, unsigned int plane, int offset, int count);
    void increasePlaneBitdepth(int plane);
    void set(int x, int y, int z, int value);
    bool testSetDataRegion(arrayWithLength<unsigned char> values, int x, int y, int z, int width, int height, int depth, int value, bool* changed);
    void setDataRegion(arrayWithLength<unsigned char> values, int x, int y, int z, int width, int height, int depth, int value, void* oldData, void* newData);
    void getDataRegion(arrayWithLength<unsigned char> values, int x, int y, int z, int width, int height, int depth, int value);
    bool isCompressed();

private:
    union DataAndCountValue
    {
        long long bits;
        struct
        {
            unsigned int sizeAndCount;
            PlaneOffsets* offsets;
        } value;
    };
    DataAndCountValue dataAndCount;

    static const unsigned int DATA_SIZE_SHIFT = 16;
};

#endif
