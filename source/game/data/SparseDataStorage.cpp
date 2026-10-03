#include "game/data/SparseDataStorage.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/XLockFreeStack.hpp"
#include "game/core/XMem.hpp"
#include <cstdlib>
#include <string.h>

long long InterlockedCompareExchangeRelease64(volatile long long* destination, long long exchange, long long comparand);

static int sPendingDeleteList;
static XLockFreeStack<unsigned char> sPendingDeleteLists[3];
static OSFastMutex sCompressLock;
static int sAllocatedBytes;

void SparseDataStorage::Array256_1Bit::init()
{
    palette[0] = 255; palette[1] = 255; palette[2] = 0; palette[3] = 0;
}

unsigned int SparseDataStorage::Array256_1Bit::set(unsigned char index, unsigned char value)
{
    const unsigned int shift = index & 7;
    const unsigned int mask = 1u << shift;
    unsigned char& packed = indices[index >> 3];
    if (value == palette[0] || palette[0] == 255)
    {
        if (palette[0] == 255) palette[0] = value;
        packed = static_cast<unsigned char>(packed & ~mask);
        return 1;
    }
    if (value == palette[1] || palette[1] == 255)
    {
        if (palette[1] == 255) palette[1] = value;
        packed = static_cast<unsigned char>((packed & ~mask) | (1u << shift));
        return 1;
    }
    return 0;
}

unsigned char SparseDataStorage::Array256_1Bit::get(unsigned char index)
{
    return palette[(indices[index >> 3] >> (index & 7)) & 1];
}

void SparseDataStorage::Array256_2Bit::init()
{
    palette[0] = 255; palette[1] = 255; palette[2] = 255; palette[3] = 255;
}

unsigned int SparseDataStorage::Array256_2Bit::set(unsigned char index, unsigned char value)
{
    const unsigned int shift = (index & 3) * 2;
    const unsigned int mask = 3u << shift;
    unsigned char& packed = indices[index >> 2];
    if (value == palette[0] || palette[0] == 255)
    {
        if (palette[0] == 255) palette[0] = value;
        packed = static_cast<unsigned char>(packed & ~mask);
        return 1;
    }
    if (value == palette[1] || palette[1] == 255)
    {
        if (palette[1] == 255) palette[1] = value;
        packed = static_cast<unsigned char>((packed & ~mask) | (1u << shift));
        return 1;
    }
    if (value == palette[2] || palette[2] == 255)
    {
        if (palette[2] == 255) palette[2] = value;
        packed = static_cast<unsigned char>((packed & ~mask) | (2u << shift));
        return 1;
    }
    if (value == palette[3] || palette[3] == 255)
    {
        if (palette[3] == 255) palette[3] = value;
        packed = static_cast<unsigned char>((packed & ~mask) | (3u << shift));
        return 1;
    }
    return 0;
}

unsigned char SparseDataStorage::Array256_2Bit::get(unsigned char index)
{
    return palette[(indices[index >> 2] >> ((index & 3) << 1)) & 3];
}

static const unsigned int PLANE_OFFSET_MASK = 0x3fff;
static const unsigned int PLANE_TYPE_SHIFT = 14;

void SparseDataStorage::PlaneOffsets::initPlane(int plane, void* data)
{
    unsigned short old = offsets[plane];
    unsigned short type = old >> PLANE_TYPE_SHIFT & 3;
    if (type)
    {
        unsigned char* address = static_cast<unsigned char*>(data);
        offsets[plane] = static_cast<unsigned short>((((address - reinterpret_cast<unsigned char*>(this)) >> 2) & PLANE_OFFSET_MASK) | (old & 0xc000));
    }
    if (type == 1)
        reinterpret_cast<Array256_1Bit*>(reinterpret_cast<unsigned char*>(this) + (offsets[plane] & PLANE_OFFSET_MASK) * 4)->init();
    else if (type == 2)
        reinterpret_cast<Array256_2Bit*>(reinterpret_cast<unsigned char*>(this) + (offsets[plane] & PLANE_OFFSET_MASK) * 4)->init();
}

unsigned int SparseDataStorage::PlaneOffsets::setData(int plane, int index, int value)
{
    unsigned short entry = offsets[plane];
    unsigned short type = entry >> PLANE_TYPE_SHIFT & 3;
    if (type == 0)
    {
        if (value != (entry & PLANE_OFFSET_MASK))
            return 0;
    }
    else
    {
        if (type == 1)
            return reinterpret_cast<Array256_1Bit*>(reinterpret_cast<unsigned char*>(this) + (entry & PLANE_OFFSET_MASK) * 4)->set(static_cast<unsigned char>(index), static_cast<unsigned char>(value));
        if (type < 3)
            return reinterpret_cast<Array256_2Bit*>(reinterpret_cast<unsigned char*>(this) + (entry & PLANE_OFFSET_MASK) * 4)->set(static_cast<unsigned char>(index), static_cast<unsigned char>(value));
        if (type != 3)
            return 0;
        const unsigned char position = static_cast<unsigned char>(index);
        const unsigned int shift = (position & 1) * 4;
        unsigned char& packed = (reinterpret_cast<unsigned char*>(this) + (entry & PLANE_OFFSET_MASK) * 4)[position >> 1];
        packed = static_cast<unsigned char>((packed & ~(15u << shift)) |
                                            (static_cast<unsigned int>(value) << shift));
    }
    return 1;
}

unsigned char SparseDataStorage::PlaneOffsets::getDataSizeFromType(int type) { return type == 1 ? 0x24 : type == 2 ? 0x44 : type == 3 ? 0x80 : 0; }
unsigned char SparseDataStorage::PlaneOffsets::getDataSize(int plane) { return getDataSizeFromType(offsets[plane] >> 14); }
unsigned int SparseDataStorage::PlaneOffsets::getData(int plane, int index)
{
    unsigned short entry = offsets[plane];
    unsigned short type = entry >> PLANE_TYPE_SHIFT & 3;
    unsigned int result = 0;
    if (type == 0)
        result = entry & PLANE_OFFSET_MASK;
    else if (type == 1)
        result = reinterpret_cast<Array256_1Bit*>(reinterpret_cast<unsigned char*>(this) + (entry & PLANE_OFFSET_MASK) * 4)->get(static_cast<unsigned char>(index));
    else if (type < 3)
        result = reinterpret_cast<Array256_2Bit*>(reinterpret_cast<unsigned char*>(this) + (entry & PLANE_OFFSET_MASK) * 4)->get(static_cast<unsigned char>(index));
    else if (type == 3)
    {
        unsigned char position = static_cast<unsigned char>(index);
        result = ((reinterpret_cast<unsigned char*>(this) + (entry & PLANE_OFFSET_MASK) * 4)[position >> 1] >> ((position & 1) << 2)) & 15;
    }
    return result;
}

unsigned int SparseDataStorage::PlaneOffsets::getValuesInPlane(int plane, char (&values)[16])
{
    unsigned int count = 0;
    for (int i = 0; i < 256; ++i)
    {
        unsigned char value = (unsigned char)getData(plane, i);
        bool found = false;
        for (unsigned int j = 0; j < count; ++j)
        {
            if ((unsigned char)values[j] == value)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            values[count++] = (char)value;
        }
    }
    return count;
}
SparseDataStorage::PlaneOffsets* SparseDataStorage::allocatePlaneOffsets(unsigned int size)
{
    sAllocatedBytes += size;
    return static_cast<PlaneOffsets*>(malloc(size));
}
void SparseDataStorage::freePlaneOffsets(PlaneOffsets* offsets, int size)
{
    sAllocatedBytes -= size;
    free(offsets);
}
long long SparseDataStorage::createDataAndCount(PlaneOffsets* offsets, int size)
{
    DataAndCountValue value = {};
    value.value.size = static_cast<unsigned int>(size);
    value.value.offsets = offsets;
    return value.bits;
}
SparseDataStorage::SparseDataStorage()
{
    PlaneOffsets* offsets = allocatePlaneOffsets(0x4080);
    unsigned char* payload = offsets->payload;
    offsets->offsets[0] = 0;
    for (int plane = 1; plane < 128; ++plane)
    {
        offsets->offsets[plane] = 0xc000;
        offsets->initPlane(plane, payload);
        payload += 0x80;
    }
    XMemSet(offsets->payload, 0, 0x3f80);
    dataAndCount.bits = createDataAndCount(offsets, 0x4080);
}
SparseDataStorage::~SparseDataStorage()
{
    PlaneOffsets* offsets = getPlaneOffsets();
    freePlaneOffsets(offsets, getPlaneOffsetsDataSize());
}
SparseDataStorage::PlaneOffsets* SparseDataStorage::getPlaneOffsets()
{
    return dataAndCount.value.offsets;
}
unsigned int SparseDataStorage::getPlaneOffsetsDataSize()
{
    return dataAndCount.value.size;
}
void SparseDataStorage::queueForDelete(unsigned char* data, int)
{
    EnterCriticalSection(&sCompressLock);
    sPendingDeleteLists[sPendingDeleteList].Push(data);
    LeaveCriticalSection(&sCompressLock);
}
void SparseDataStorage::updateDataAndCount(PlaneOffsets* offsets, int size)
{
    const long long replacement = createDataAndCount(offsets, size);
    const int oldSize = getPlaneOffsetsDataSize();
    long long previous;
    do { previous = dataAndCount.bits; }
    while (InterlockedCompareExchangeRelease64(&dataAndCount.bits, replacement, previous) != previous);
    DataAndCountValue oldValue = {};
    oldValue.bits = previous;
    queueForDelete(reinterpret_cast<unsigned char*>(oldValue.value.offsets), oldSize);
}
unsigned int SparseDataStorage::get(int x, int y, int z)
{
    return getPlaneOffsets()->getData(y, x * 16 + z);
}
unsigned int SparseDataStorage::tryUpdateDataAndCount(PlaneOffsets* offsets, int size)
{
    const int oldSize = getPlaneOffsetsDataSize();
    const long long previous = dataAndCount.bits;
    const long long replacement = createDataAndCount(offsets, size);
    if (InterlockedCompareExchangeRelease64(&dataAndCount.bits, replacement, previous) != previous)
        return 0;
    DataAndCountValue oldValue = {};
    oldValue.bits = previous;
    queueForDelete(reinterpret_cast<unsigned char*>(oldValue.value.offsets), oldSize);
    return 1;
}
void SparseDataStorage::staticCtor()
{
    InitializeCriticalSection(&sCompressLock);
}

unsigned int SparseDataStorage::compress()
{
    EnterCriticalSection(&sCompressLock);

    PlaneOffsets* sourceOffsets = getPlaneOffsets();
    unsigned int currentSize = getPlaneOffsetsDataSize();
    if (currentSize < 0x3f80)
    {
        LeaveCriticalSection(&sCompressLock);
        return currentSize;
    }

    unsigned short localOffsets[128];
    unsigned int payloadSize = 0;
    for (int plane = 0; plane < 128; ++plane)
    {
        char uniqueValues[16];
        unsigned int valueCount = sourceOffsets->getValuesInPlane(plane, uniqueValues);
        if (valueCount < 2)
        {
            localOffsets[plane] = static_cast<unsigned short>(uniqueValues[0]) & 0x3fff;
        }
        else if (valueCount == 2)
        {
            localOffsets[plane] = 0x4000;
            payloadSize += 0x24;
        }
        else if (valueCount <= 4)
        {
            localOffsets[plane] = 0x8000;
            payloadSize += 0x44;
        }
        else
        {
            localOffsets[plane] = 0xc000;
            payloadSize += 0x80;
        }
    }

    const unsigned int newSize = payloadSize + 0x100;
    currentSize = getPlaneOffsetsDataSize();
    if (newSize != currentSize)
    {
        PlaneOffsets* destination = allocatePlaneOffsets(newSize);
        unsigned char* dataCursor = destination->payload;
        XMemCpy(destination, localOffsets, 0x100);
        for (int plane = 0; plane < 128; ++plane)
        {
            destination->initPlane(plane, dataCursor);
            for (int index = 0; index < 256; ++index)
            {
                destination->setData(plane, index, static_cast<unsigned char>(sourceOffsets->getData(plane, index)));
            }
            dataCursor += destination->getDataSize(plane);
        }

        if (tryUpdateDataAndCount(destination, static_cast<int>(newSize)) == 0)
            queueForDelete(reinterpret_cast<unsigned char*>(destination), static_cast<int>(newSize));
        LeaveCriticalSection(&sCompressLock);
        return newSize;
    }

    LeaveCriticalSection(&sCompressLock);
    return currentSize;
}
void SparseDataStorage::tick()
{
    XLockFreeStack<unsigned char>& pending = sPendingDeleteLists[(sPendingDeleteList + 1) % 3];
    unsigned char* data;
    do
    {
        data = pending.Pop();
        if (data != NULL)
            freePlaneOffsets(static_cast<PlaneOffsets*>(static_cast<void*>(data)), 0);
    }
    while (data != NULL);
    sPendingDeleteList = (sPendingDeleteList + 1) % 3;
}
void SparseDataStorage::write(DataOutputStream* stream)
{
    PlaneOffsets* planeOffsets = getPlaneOffsets();
    unsigned int nonEmpty = 0;
    for (int plane = 0; plane < 128; ++plane)
        if (planeOffsets->offsets[plane] != 0)
            ++nonEmpty;

    const unsigned int blockSize = 0x80;
    const unsigned int totalSize = blockSize * (nonEmpty + 1);
    unsigned char* encoded = new unsigned char[totalSize];
    unsigned int block = 0;
    for (int plane = 0; plane < 128; ++plane)
    {
        if (planeOffsets->offsets[plane] == 0)
        {
            encoded[plane] = 0x80;
            continue;
        }
        encoded[plane] = (unsigned char)block++;
        unsigned char* destination = &encoded[blockSize * (encoded[plane] + 1)];
        for (int group = 0; group < 16; ++group)
        {
            const int first = group * 16;
            for (int pair = 0; pair < 8; ++pair)
            {
                unsigned char low = static_cast<unsigned char>(planeOffsets->getData(plane, first + pair * 2));
                unsigned char high = static_cast<unsigned char>(planeOffsets->getData(plane, first + pair * 2 + 1));
                destination[group * 8 + pair] = (unsigned char)(low | (high << 4));
            }
        }
    }
    stream->writeInt((int)nonEmpty);
    arrayWithLength<unsigned char> output(encoded, totalSize);
    stream->write(output);
    delete[] encoded;
}
void SparseDataStorage::read(DataInputStream* stream)
{
    const unsigned int nonEmpty = (unsigned int)stream->readInt();
    const unsigned int inputSize = 0x80 * (nonEmpty + 1);
    unsigned char* encoded = static_cast<unsigned char*>(malloc(inputSize));
    arrayWithLength<unsigned char> input(encoded, inputSize);
    stream->readFully(input);

    const unsigned int requiredSize = nonEmpty * 0x80 + 0x100;
    PlaneOffsets* destination;
    bool replaced = false;
    if (getPlaneOffsetsDataSize() != requiredSize)
    {
        destination = allocatePlaneOffsets(requiredSize);
        replaced = true;
    }
    else
    {
        destination = getPlaneOffsets();
    }
    unsigned int payload = 0x100;
    for (int plane = 0; plane < 128; ++plane)
    {
        const unsigned char directory = encoded[plane];
        if (directory == 0x80)
        {
            destination->offsets[plane] = 0;
            continue;
        }
        destination->offsets[plane] = static_cast<unsigned short>(0xc000 | ((payload >> 2) & 0x3fff));
        const unsigned char* source = &encoded[0x80 + directory * 0x80];
        for (int group = 0; group < 16; ++group)
            for (int pair = 0; pair < 8; ++pair)
            {
                unsigned char packed = source[group * 8 + pair];
                destination->setData(plane, group * 16 + pair * 2, packed & 15);
                destination->setData(plane, group * 16 + pair * 2 + 1, packed >> 4);
            }
        payload += 0x80;
    }
    if (replaced)
        updateDataAndCount(destination, (int)requiredSize);
    compress();
    free(encoded);
}
