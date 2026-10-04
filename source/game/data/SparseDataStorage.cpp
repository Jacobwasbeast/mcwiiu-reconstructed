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
    palette[0] = -1; palette[1] = -1; palette[2] = 0; palette[3] = 0;
}

unsigned int SparseDataStorage::Array256_1Bit::set(unsigned char index, unsigned char value)
{
    unsigned int result;
    const int slot = index >> 3;
    if (value == palette[0])
    {
        result = 1;
        indices[slot] &= ~(1 << (index & 7));
    }
    else if (palette[0] == -1)
    {
        palette[0] = value;
        result = 1;
        indices[slot] &= ~(1 << (index & 7));
    }
    else if (value == palette[1])
    {
        result = 1;
        indices[slot] &= ~(1 << (index & 7));
        indices[slot] |= 1 << (index & 7);
    }
    else if (palette[1] == -1)
    {
        result = 1;
        palette[1] = value;
        indices[slot] &= ~(1 << (index & 7));
        indices[slot] |= 1 << (index & 7);
    }
    else
    {
        result = 0;
    }
    return result;
}

unsigned char SparseDataStorage::Array256_1Bit::get(unsigned char index)
{
    return palette[(indices[index >> 3] >> (index & 7)) & 1];
}

void SparseDataStorage::Array256_2Bit::init()
{
    palette[0] = -1; palette[1] = -1; palette[2] = -1; palette[3] = -1;
}

unsigned int SparseDataStorage::Array256_2Bit::set(unsigned char index, unsigned char value)
{
    const int slot = index >> 2;
    if (value == palette[0])
    {
        indices[slot] &= ~(3 << ((index & 3) << 1));
        return 1;
    }
    if (palette[0] == -1)
    {
        palette[0] = value;
        indices[slot] &= ~(3 << ((index & 3) << 1));
        return 1;
    }
    if (value == palette[1])
    {
        const int shift = (index & 3) << 1;
        indices[slot] = static_cast<unsigned char>((indices[slot] & ~(3 << shift)) | (1 << shift));
        return 1;
    }
    if (palette[1] == -1)
    {
        palette[1] = value;
        const int shift = (index & 3) << 1;
        indices[slot] = static_cast<unsigned char>((indices[slot] & ~(3 << shift)) | (1 << shift));
        return 1;
    }
    if (value == palette[2])
    {
        const int shift = (index & 3) << 1;
        indices[slot] = static_cast<unsigned char>((indices[slot] & ~(3 << shift)) | (2 << shift));
        return 1;
    }
    if (palette[2] == -1)
    {
        palette[2] = value;
        const int shift = (index & 3) << 1;
        indices[slot] = static_cast<unsigned char>((indices[slot] & ~(3 << shift)) | (2 << shift));
        return 1;
    }
    if (value == palette[3])
    {
        indices[slot] &= ~(3 << ((index & 3) << 1));
        indices[slot] |= 3 << ((index & 3) << 1);
        return 1;
    }
    if (palette[3] == -1)
    {
        palette[3] = value;
        indices[slot] &= ~(3 << ((index & 3) << 1));
        indices[slot] |= 3 << ((index & 3) << 1);
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
    unsigned int entry = offsets[plane];
    const unsigned int type = offsets[plane] >> PLANE_TYPE_SHIFT & 3;
    if (type != 0)
    {
        unsigned char* address = static_cast<unsigned char*>(data);
        entry = (offsets[plane] & ~PLANE_OFFSET_MASK) | (static_cast<unsigned int>(address - reinterpret_cast<unsigned char*>(this)) >> 2 & PLANE_OFFSET_MASK);
        offsets[plane] = static_cast<unsigned short>(entry);
    }
    switch (type)
    {
    case 1:
        reinterpret_cast<Array256_1Bit*>(reinterpret_cast<unsigned char*>(this) + (entry & PLANE_OFFSET_MASK) * 4)->init();
        break;
    case 2:
        reinterpret_cast<Array256_2Bit*>(reinterpret_cast<unsigned char*>(this) + (entry & PLANE_OFFSET_MASK) * 4)->init();
        break;
    }
}

unsigned int SparseDataStorage::PlaneOffsets::setData(int plane, int index, int value)
{
    unsigned int entry = offsets[plane];
    unsigned int type = offsets[plane] >> PLANE_TYPE_SHIFT & 3;
    unsigned char* data = reinterpret_cast<unsigned char*>(this) + (entry & PLANE_OFFSET_MASK) * 4;
    unsigned int result = 1;
    switch (type)
    {
    case 0:
        if (value != (entry & PLANE_OFFSET_MASK))
            result = 0;
        break;
    case 1:
        result = reinterpret_cast<Array256_1Bit*>(data)->set(static_cast<unsigned char>(index), static_cast<unsigned char>(value));
        break;
    case 2:
        result = reinterpret_cast<Array256_2Bit*>(data)->set(static_cast<unsigned char>(index), static_cast<unsigned char>(value));
        break;
    case 3:
    {
        const unsigned char position = static_cast<unsigned char>(index);
        const unsigned int shift = (position & 1) << 2;
        data[position >> 1] = static_cast<unsigned char>((data[position >> 1] & ~(15u << shift)) | (static_cast<unsigned int>(value) << shift));
        break;
    }
    default:
        result = 0;
        break;
    }
    return result;
}

unsigned char SparseDataStorage::PlaneOffsets::getDataSizeFromType(int type)
{
    static const unsigned char sizes[] = { 0x24, 0x44, 0x80 };
    const unsigned int planeType = static_cast<unsigned int>(type);
    if (planeType != 0 && planeType <= 3)
        return sizes[planeType - 1];
    return 0;
}
unsigned char SparseDataStorage::PlaneOffsets::getDataSize(int plane) { return getDataSizeFromType(offsets[plane] >> 14); }
unsigned int SparseDataStorage::PlaneOffsets::getData(int plane, int index)
{
    unsigned int entry = offsets[plane];
    unsigned int type = offsets[plane] >> PLANE_TYPE_SHIFT & 3;
    unsigned int result = 0;
    switch (type)
    {
    case 0:
        result = entry & PLANE_OFFSET_MASK;
        break;
    case 1:
        result = reinterpret_cast<Array256_1Bit*>(reinterpret_cast<unsigned char*>(this) + (entry & PLANE_OFFSET_MASK) * 4)->get(static_cast<unsigned char>(index));
        break;
    case 2:
        result = reinterpret_cast<Array256_2Bit*>(reinterpret_cast<unsigned char*>(this) + (entry & PLANE_OFFSET_MASK) * 4)->get(static_cast<unsigned char>(index));
        break;
    case 3:
    {
        unsigned char position = static_cast<unsigned char>(index);
        unsigned char* nibbles = reinterpret_cast<unsigned char*>(this) + (entry & PLANE_OFFSET_MASK) * 4;
        result = (nibbles[position >> 1] >> ((position & 1) << 2)) & 15;
        break;
    }
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
    value.value.sizeAndCount = static_cast<unsigned int>(size) << DATA_SIZE_SHIFT;
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
        offsets->offsets[plane] |= 0xc000;
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
    return dataAndCount.value.sizeAndCount >> DATA_SIZE_SHIFT;
}
void SparseDataStorage::queueForDelete(unsigned char* data, int)
{
    sPendingDeleteLists[sPendingDeleteList].Push(data);
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
    int block = 0;
    unsigned char* destination = encoded + blockSize;
    for (int plane = 0; plane < 128; ++plane)
    {
        if (planeOffsets->offsets[plane] == 0)
        {
            encoded[plane] = 0x80;
            continue;
        }
        encoded[plane] = static_cast<unsigned char>(block++);
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
        destination += blockSize;
    }
    stream->writeInt((int)nonEmpty);
    stream->write(arrayWithLength<unsigned char>(encoded, totalSize));
    delete[] encoded;
}
void SparseDataStorage::read(DataInputStream* stream)
{
    const unsigned int nonEmpty = (unsigned int)stream->readInt();
    const unsigned int inputSize = 0x80 * (nonEmpty + 1);
    unsigned char* encoded = static_cast<unsigned char*>(malloc(inputSize));
    stream->readFully(arrayWithLength<unsigned char>(encoded, inputSize));

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
    unsigned char* payload = reinterpret_cast<unsigned char*>(destination) + 0x100;
    for (int plane = 0; plane < 128; ++plane)
    {
        const unsigned char directory = encoded[plane];
        if (directory == 0x80)
        {
            destination->offsets[plane] = 0;
            payload += destination->getDataSize(plane);
            continue;
        }
        destination->offsets[plane] = static_cast<unsigned short>(0xc000 | (((payload - reinterpret_cast<unsigned char*>(destination)) >> 2) & 0x3fff));
        const unsigned char* source = &encoded[0x80 + directory * 0x80];
        for (int group = 0; group < 16; ++group)
            for (int pair = 0; pair < 8; ++pair)
            {
                unsigned char packed = source[group * 8 + pair];
                destination->setData(plane, group * 16 + pair * 2, packed & 15);
                destination->setData(plane, group * 16 + pair * 2 + 1, packed >> 4);
            }
        payload += destination->getDataSize(plane);
    }
    if (replaced)
        updateDataAndCount(destination, (int)requiredSize);
    compress();
    free(encoded);
}
