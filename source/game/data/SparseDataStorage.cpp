#include "game/data/SparseDataStorage.hpp"

#include <cstdlib>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/XLockFreeStack.hpp"

void* XMemSet(void* destination, int value, unsigned int size);
void* XMemCpy(void* destination, const void* source, unsigned int size);
long long InterlockedCompareExchangeRelease64(volatile long long* destination, long long exchange, long long comparand);

static const int CHUNK_WIDTH = 16;
static const int PLANE_COUNT = 128;
static const int PLANE_VALUES = 256;
static const int PLANE_BYTES = 128;
static const int PLANE_1BIT_BYTES = 36;
static const int PLANE_2BIT_BYTES = 68;
static const int PLANE_OFFSET_BYTES = PLANE_COUNT * 2;
static const int PLANE_TYPE_SHIFT = 14;
static const unsigned short PLANE_OFFSET_MASK = 0x3fff;
static const int PLANE_TYPE_VALUE = 0;
static const int PLANE_TYPE_1BIT = 1;
static const int PLANE_TYPE_2BIT = 2;
static const int PLANE_TYPE_4BIT = 3;
static const int PLANE_TYPE_COUNT = 4;
static const unsigned int PLANE_EMPTY = 0x80;
static const int PENDING_DELETE_LIST_COUNT = 3;
static const int VALUES_IN_PLANE_MAX = 16;

static const unsigned char sPlaneDataSizes[PLANE_TYPE_COUNT] =
{
    0,
    PLANE_1BIT_BYTES,
    PLANE_2BIT_BYTES,
    PLANE_BYTES
};

static int sAllocatedBytes = 0;

static OSFastMutex sCompressLock;

static int sPendingDeleteList;
static XLockFreeStack<unsigned char> sPendingDeleteLists[PENDING_DELETE_LIST_COUNT];

SparseDataStorage::PlaneOffsets* SparseDataStorage::allocatePlaneOffsets(unsigned int dataSize)
{
    sAllocatedBytes += dataSize;

    return (PlaneOffsets*)malloc(dataSize);
}

void SparseDataStorage::freePlaneOffsets(PlaneOffsets* planeOffsets, int dataSize)
{
    sAllocatedBytes -= dataSize;

    free(planeOffsets);
}

long long SparseDataStorage::createDataAndCount(PlaneOffsets* planeOffsets, int dataSize)
{
    return ((long long)dataSize << 48) | (unsigned int)planeOffsets;
}

SparseDataStorage::PlaneOffsets* SparseDataStorage::getPlaneOffsets()
{
    return (PlaneOffsets*)(unsigned int)mDataAndCount;
}

unsigned int SparseDataStorage::getPlaneOffsetsDataSize()
{
    return (unsigned int)(mDataAndCount >> 48) & 0xffff;
}

SparseDataStorage::SparseDataStorage()
{
    int dataSize = PLANE_OFFSET_BYTES + (PLANE_COUNT - 1) * PLANE_BYTES;

    PlaneOffsets* planeOffsets = allocatePlaneOffsets(dataSize);

    planeOffsets->mOffsets[0] = 0;

    unsigned char* planeData = (unsigned char*)planeOffsets + PLANE_OFFSET_BYTES;

    for (int plane = 1; plane < PLANE_COUNT; plane++)
    {
        planeOffsets->mOffsets[plane] |= (unsigned short)(PLANE_TYPE_4BIT << PLANE_TYPE_SHIFT);

        planeOffsets->initPlane(plane, planeData);

        planeData += PLANE_BYTES;
    }

    XMemSet((unsigned char*)planeOffsets + PLANE_OFFSET_BYTES, 0, (PLANE_COUNT - 1) * PLANE_BYTES);

    mDataAndCount = createDataAndCount(planeOffsets, dataSize);
}

SparseDataStorage::~SparseDataStorage()
{
    PlaneOffsets* planeOffsets = getPlaneOffsets();

    freePlaneOffsets(planeOffsets, (int)getPlaneOffsetsDataSize());
}

void SparseDataStorage::queueForDelete(unsigned char* data, int dataSize)
{
    sPendingDeleteLists[sPendingDeleteList].Push(data);
}

void SparseDataStorage::updateDataAndCount(PlaneOffsets* planeOffsets, int dataSize)
{
    long long dataAndCount = createDataAndCount(planeOffsets, dataSize);

    int oldDataSize = (int)getPlaneOffsetsDataSize();

    long long oldDataAndCount;
    unsigned char* oldData;

    do
    {
        oldDataAndCount = mDataAndCount;
        oldData = (unsigned char*)(unsigned int)oldDataAndCount;
    }
    while (InterlockedCompareExchangeRelease64(&mDataAndCount, dataAndCount, oldDataAndCount) != oldDataAndCount);

    queueForDelete(oldData, oldDataSize);
}

bool SparseDataStorage::tryUpdateDataAndCount(PlaneOffsets* planeOffsets, int dataSize)
{
    PlaneOffsets* oldPlaneOffsets = getPlaneOffsets();

    int oldDataSize = (int)getPlaneOffsetsDataSize();

    long long oldDataAndCount = mDataAndCount;

    if (InterlockedCompareExchangeRelease64(&mDataAndCount, createDataAndCount(planeOffsets, dataSize), oldDataAndCount) != oldDataAndCount)
    {
        return false;
    }

    queueForDelete((unsigned char*)oldPlaneOffsets, oldDataSize);

    return true;
}

int SparseDataStorage::get(int x, int y, int z)
{
    PlaneOffsets* planeOffsets = getPlaneOffsets();

    return planeOffsets->getData(y, x * CHUNK_WIDTH + z);
}

void SparseDataStorage::staticCtor()
{
    InitializeCriticalSection(&sCompressLock);
}

unsigned int SparseDataStorage::compress()
{
    EnterCriticalSection(&sCompressLock);

    PlaneOffsets* oldPlaneOffsets = getPlaneOffsets();

    unsigned int oldDataSize = getPlaneOffsetsDataSize();

    if (oldDataSize < (unsigned int)((PLANE_COUNT - 1) * PLANE_BYTES))
    {
        LeaveCriticalSection(&sCompressLock);

        return oldDataSize;
    }

    char values[VALUES_IN_PLANE_MAX];
    unsigned short offsets[PLANE_COUNT];

    int planeBytes = 0;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        int valueCount = oldPlaneOffsets->getValuesInPlane(plane, values);

        if (valueCount > 4)
        {
            offsets[plane] = (unsigned short)((offsets[plane] & PLANE_OFFSET_MASK) | (PLANE_TYPE_4BIT << PLANE_TYPE_SHIFT));

            planeBytes += PLANE_BYTES;
        }
        else if (valueCount > 2)
        {
            offsets[plane] = (unsigned short)((offsets[plane] & PLANE_OFFSET_MASK) | (PLANE_TYPE_2BIT << PLANE_TYPE_SHIFT));

            planeBytes += PLANE_2BIT_BYTES;
        }
        else if (valueCount > 1)
        {
            offsets[plane] = (unsigned short)((offsets[plane] & PLANE_OFFSET_MASK) | (PLANE_TYPE_1BIT << PLANE_TYPE_SHIFT));

            planeBytes += PLANE_1BIT_BYTES;
        }
        else
        {
            offsets[plane] = (unsigned short)(values[0] & PLANE_OFFSET_MASK);
        }
    }

    unsigned int dataSize = planeBytes + PLANE_OFFSET_BYTES;

    if (dataSize == getPlaneOffsetsDataSize())
    {
        LeaveCriticalSection(&sCompressLock);

        return dataSize;
    }

    PlaneOffsets* planeOffsets = allocatePlaneOffsets(dataSize);

    unsigned char* planeData = (unsigned char*)planeOffsets + PLANE_OFFSET_BYTES;

    XMemCpy(planeOffsets, offsets, PLANE_OFFSET_BYTES);

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        planeOffsets->initPlane(plane, planeData);

        for (int index = 0; index < PLANE_VALUES; index++)
        {
            char value = (char)oldPlaneOffsets->getData(plane, index);

            planeOffsets->setData(plane, index, value);
        }

        planeData += planeOffsets->getDataSize(plane);
    }

    if (!tryUpdateDataAndCount(planeOffsets, dataSize))
    {
        queueForDelete((unsigned char*)planeOffsets, dataSize);
    }

    LeaveCriticalSection(&sCompressLock);

    return dataSize;
}

void SparseDataStorage::tick()
{
    XLockFreeStack<unsigned char>& pending = sPendingDeleteLists[(sPendingDeleteList + 1) % PENDING_DELETE_LIST_COUNT];

    unsigned char* data = pending.Pop();

    while (data != NULL)
    {
        freePlaneOffsets((PlaneOffsets*)data, 0);

        data = pending.Pop();
    }

    sPendingDeleteList = (sPendingDeleteList + 1) % PENDING_DELETE_LIST_COUNT;
}

void SparseDataStorage::write(DataOutputStream* out)
{
    PlaneOffsets* planeOffsets = getPlaneOffsets();

    int planeCount = 0;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        if (planeOffsets->mOffsets[plane] != 0)
        {
            planeCount++;
        }
    }

    int size = planeCount * PLANE_BYTES + PLANE_BYTES;

    unsigned char* buffer = new unsigned char[size];

    unsigned char* planeData = buffer + PLANE_BYTES;

    unsigned char index = 0;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        if (planeOffsets->mOffsets[plane] == 0)
        {
            buffer[plane] = (unsigned char)PLANE_EMPTY;
        }
        else
        {
            buffer[plane] = index;

            index++;

            for (int value = 0; value < PLANE_VALUES; value += 2)
            {
                planeData[value >> 1] = (unsigned char)(planeOffsets->getData(plane, value) | (planeOffsets->getData(plane, value + 1) << 4));
            }

            planeData += PLANE_BYTES;
        }
    }

    out->writeInt(planeCount);

    arrayWithLength<unsigned char> data;

    data.data = buffer;
    data.length = size;

    out->write(data);

    delete[] buffer;
}

void SparseDataStorage::read(DataInputStream* in)
{
    int planeCount = in->readInt();

    int size = planeCount * PLANE_BYTES + PLANE_BYTES;

    unsigned char* buffer = (unsigned char*)malloc(size);

    arrayWithLength<unsigned char> raw;

    raw.data = buffer;
    raw.length = size;

    in->readFully(raw);

    unsigned int dataSize = planeCount * PLANE_BYTES + PLANE_OFFSET_BYTES;

    bool allocated = false;

    PlaneOffsets* planeOffsets;

    if (getPlaneOffsetsDataSize() == dataSize)
    {
        planeOffsets = getPlaneOffsets();
    }
    else
    {
        planeOffsets = allocatePlaneOffsets(dataSize);

        allocated = true;
    }

    unsigned char* planeData = (unsigned char*)planeOffsets + PLANE_OFFSET_BYTES;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        if (buffer[plane] == PLANE_EMPTY)
        {
            planeOffsets->mOffsets[plane] = 0;
        }
        else
        {
            unsigned int offset = (unsigned int)(planeData - (unsigned char*)planeOffsets) >> 2;

            planeOffsets->mOffsets[plane] = (unsigned short)((offset & PLANE_OFFSET_MASK) | (PLANE_TYPE_4BIT << PLANE_TYPE_SHIFT));

            const unsigned char* source = buffer + PLANE_BYTES + buffer[plane] * PLANE_BYTES;

            for (int value = 0; value < PLANE_VALUES; value += 2)
            {
                unsigned char packed = source[value >> 1];

                planeOffsets->setData(plane, value, packed & 0xf);
                planeOffsets->setData(plane, value + 1, packed >> 4);
            }
        }

        planeData += planeOffsets->getDataSize(plane);
    }

    if (allocated)
    {
        updateDataAndCount(planeOffsets, dataSize);
    }

    compress();

    free(buffer);
}

void SparseDataStorage::Array256_1Bit::init()
{
    mValues[0] = -1;
    mValues[1] = -1;
    mValues[2] = 0;
    mValues[3] = 0;
}

bool SparseDataStorage::Array256_1Bit::set(unsigned char index, unsigned char value)
{
    int position = index >> 3;
    int shift = index & 7;

    if (value == mValues[0])
    {
        mBits[position] = (unsigned char)(mBits[position] & ~(1 << shift));

        return true;
    }

    if (mValues[0] == -1)
    {
        mValues[0] = (char)value;

        mBits[position] = (unsigned char)(mBits[position] & ~(1 << shift));

        return true;
    }

    if (value == mValues[1])
    {
        mBits[position] = (unsigned char)((mBits[position] & ~(1 << shift)) | (1 << shift));

        return true;
    }

    if (mValues[1] == -1)
    {
        mValues[1] = (char)value;

        mBits[position] = (unsigned char)((mBits[position] & ~(1 << shift)) | (1 << shift));

        return true;
    }

    return false;
}

unsigned char SparseDataStorage::Array256_1Bit::get(unsigned char index)
{
    return mValues[(mBits[index >> 3] >> (index & 7)) & 1];
}

void SparseDataStorage::Array256_2Bit::init()
{
    mValues[0] = -1;
    mValues[1] = -1;
    mValues[2] = -1;
    mValues[3] = -1;
}

bool SparseDataStorage::Array256_2Bit::set(unsigned char index, unsigned char value)
{
    int position = index >> 2;
    int shift = (index & 3) << 1;

    if (value == mValues[0])
    {
        mBits[position] = (unsigned char)(mBits[position] & ~(3 << shift));

        return true;
    }

    if (mValues[0] == -1)
    {
        mValues[0] = (char)value;

        mBits[position] = (unsigned char)(mBits[position] & ~(3 << shift));

        return true;
    }

    if (value == mValues[1])
    {
        mBits[position] = (unsigned char)((mBits[position] & ~(3 << shift)) | (1 << shift));

        return true;
    }

    if (mValues[1] == -1)
    {
        mValues[1] = (char)value;

        mBits[position] = (unsigned char)((mBits[position] & ~(3 << shift)) | (1 << shift));

        return true;
    }

    if (value == mValues[2])
    {
        mBits[position] = (unsigned char)((mBits[position] & ~(3 << shift)) | (2 << shift));

        return true;
    }

    if (mValues[2] == -1)
    {
        mValues[2] = (char)value;

        mBits[position] = (unsigned char)((mBits[position] & ~(3 << shift)) | (2 << shift));

        return true;
    }

    if (value == mValues[3])
    {
        mBits[position] = (unsigned char)((mBits[position] & ~(3 << shift)) | (3 << shift));

        return true;
    }

    if (mValues[3] == -1)
    {
        mValues[3] = (char)value;

        mBits[position] = (unsigned char)((mBits[position] & ~(3 << shift)) | (3 << shift));

        return true;
    }

    return false;
}

unsigned char SparseDataStorage::Array256_2Bit::get(unsigned char index)
{
    return mValues[(mBits[index >> 2] >> ((index & 3) << 1)) & 3];
}

void SparseDataStorage::PlaneOffsets::initPlane(int plane, void* planeData)
{
    unsigned short offset = mOffsets[plane];

    int type = offset >> PLANE_TYPE_SHIFT;

    if (type != PLANE_TYPE_VALUE)
    {
        unsigned int dataOffset = (unsigned int)((unsigned char*)planeData - (unsigned char*)this) >> 2;

        offset = (unsigned short)((dataOffset & PLANE_OFFSET_MASK) | (offset & ~PLANE_OFFSET_MASK));

        mOffsets[plane] = offset;
    }

    if (type == PLANE_TYPE_1BIT)
    {
        ((Array256_1Bit*)((unsigned char*)this + (offset & PLANE_OFFSET_MASK) * 4))->init();
    }
    else if (type == PLANE_TYPE_2BIT)
    {
        ((Array256_2Bit*)((unsigned char*)this + (offset & PLANE_OFFSET_MASK) * 4))->init();
    }
}

bool SparseDataStorage::PlaneOffsets::setData(int plane, int index, int value)
{
    unsigned short offset = mOffsets[plane];

    int type = offset >> PLANE_TYPE_SHIFT;

    if (type == PLANE_TYPE_VALUE)
    {
        return value == (offset & PLANE_OFFSET_MASK);
    }

    if (type == PLANE_TYPE_1BIT)
    {
        return ((Array256_1Bit*)((unsigned char*)this + (offset & PLANE_OFFSET_MASK) * 4))->set((unsigned char)index, (unsigned char)value);
    }

    if (type == PLANE_TYPE_2BIT)
    {
        return ((Array256_2Bit*)((unsigned char*)this + (offset & PLANE_OFFSET_MASK) * 4))->set((unsigned char)index, (unsigned char)value);
    }

    if (type == PLANE_TYPE_4BIT)
    {
        unsigned char* planeData = (unsigned char*)this + (offset & PLANE_OFFSET_MASK) * 4;

        unsigned char position = (unsigned char)index;

        int shift = (position & 1) << 2;

        planeData[position >> 1] = (unsigned char)((planeData[position >> 1] & ~(0xf << shift)) | (value << shift));

        return true;
    }

    return false;
}

unsigned char SparseDataStorage::PlaneOffsets::getDataSizeFromType(int type)
{
    if (type != PLANE_TYPE_VALUE && type < PLANE_TYPE_COUNT)
    {
        return sPlaneDataSizes[type];
    }

    return 0;
}

unsigned char SparseDataStorage::PlaneOffsets::getDataSize(int plane)
{
    return getDataSizeFromType(mOffsets[plane] >> PLANE_TYPE_SHIFT);
}

int SparseDataStorage::PlaneOffsets::getData(int plane, int index)
{
    unsigned short offset = mOffsets[plane];

    int type = offset >> PLANE_TYPE_SHIFT;

    int value = 0;

    if (type == PLANE_TYPE_VALUE)
    {
        value = offset & PLANE_OFFSET_MASK;
    }
    else if (type == PLANE_TYPE_1BIT)
    {
        value = ((Array256_1Bit*)((unsigned char*)this + (offset & PLANE_OFFSET_MASK) * 4))->get((unsigned char)index);
    }
    else if (type == PLANE_TYPE_2BIT)
    {
        value = ((Array256_2Bit*)((unsigned char*)this + (offset & PLANE_OFFSET_MASK) * 4))->get((unsigned char)index);
    }
    else if (type == PLANE_TYPE_4BIT)
    {
        const unsigned char* planeData = (const unsigned char*)this + (offset & PLANE_OFFSET_MASK) * 4;

        unsigned char position = (unsigned char)index;

        value = (planeData[position >> 1] >> ((position & 1) << 2)) & 0xf;
    }

    return value;
}

int SparseDataStorage::PlaneOffsets::getValuesInPlane(int plane, char (&values)[16])
{
    int valueCount = 0;

    for (int index = 0; index < PLANE_VALUES; index++)
    {
        char value = (char)getData(plane, index);

        bool found = false;

        if (index != 0)
        {
            for (int i = 0; i < valueCount; i++)
            {
                if (value == values[i])
                {
                    found = true;

                    break;
                }
            }
        }

        if (!found)
        {
            values[valueCount] = value;

            valueCount++;
        }
    }

    return valueCount;
}
