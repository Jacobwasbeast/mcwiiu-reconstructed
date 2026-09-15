#include "game/data/SparseDataStorage.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/XLockFreeStack.hpp"
#include <cstdlib>
#include <string.h>

void* XMemCpy(void* destination, const void* source, unsigned int size);
void* XMemSet(void* destination, int value, unsigned int size);
long long InterlockedCompareExchangeRelease64(volatile long long* destination, long long exchange, long long comparand);

static int sPendingDeleteList;
static XLockFreeStack<unsigned char> sPendingDeleteLists[3];
static OSFastMutex sCompressLock;
static int sAllocatedBytes;

void SparseDataStorage::Array256_1Bit::init()
{
    bytes[32] = 255; bytes[33] = 255; bytes[34] = 0; bytes[35] = 0;
}

unsigned int SparseDataStorage::Array256_1Bit::set(unsigned char index, unsigned char value)
{
    const unsigned int shift = index & 7;
    const unsigned int mask = 1u << shift;
    unsigned char& packed = bytes[index >> 3];
    for (unsigned int paletteIndex = 0; paletteIndex < 2; ++paletteIndex)
    {
        unsigned char& paletteValue = bytes[32 + paletteIndex];
        if (value != static_cast<signed char>(paletteValue) && paletteValue != 255)
            continue;
        if (paletteValue == 255)
            paletteValue = value;
        packed = static_cast<unsigned char>((packed & ~mask) | (paletteIndex << shift));
        return 1;
    }
    return 0;
}

unsigned char SparseDataStorage::Array256_1Bit::get(unsigned char index)
{
    return bytes[32 + ((bytes[index >> 3] >> (index & 7)) & 1)];
}

void SparseDataStorage::Array256_2Bit::init()
{
    bytes[64] = 255; bytes[65] = 255; bytes[66] = 255; bytes[67] = 255;
}

unsigned int SparseDataStorage::Array256_2Bit::set(unsigned char index, unsigned char value)
{
    const unsigned int shift = (index & 3) * 2;
    const unsigned int mask = 3u << shift;
    unsigned char& packed = bytes[index >> 2];
    for (unsigned int paletteIndex = 0; paletteIndex < 4; ++paletteIndex)
    {
        unsigned char& paletteValue = bytes[64 + paletteIndex];
        if (value != static_cast<signed char>(paletteValue) && paletteValue != 255)
            continue;
        if (paletteValue == 255)
            paletteValue = value;
        packed = static_cast<unsigned char>((packed & ~mask) | (paletteIndex << shift));
        return 1;
    }
    return 0;
}

unsigned char SparseDataStorage::Array256_2Bit::get(unsigned char index)
{
    return bytes[64 + ((bytes[index >> 2] >> ((index & 3) << 1)) & 3)];
}

void SparseDataStorage::PlaneOffsets::initPlane(int plane, void* data)
{
    unsigned short old = offsets[plane]; unsigned int type = old >> 14;
    if (type) offsets[plane] = (unsigned short)(((static_cast<unsigned char*>(data) - (unsigned char*)this) >> 2 & 0x3fff) | (old & 0xc000));
    if (type == 1) static_cast<Array256_1Bit*>(data)->init();
    else if (type == 2) static_cast<Array256_2Bit*>(data)->init();
}

unsigned int SparseDataStorage::PlaneOffsets::setData(int plane, int index, int value)
{
    unsigned short entry = offsets[plane]; unsigned int type = entry >> 14;
    if (!type) return value == (entry & 0x3fff);
    unsigned char* p = (unsigned char*)this + (entry & 0x3fff) * 4;
    if (type == 1) return static_cast<Array256_1Bit*>(static_cast<void*>(p))->set((unsigned char)index, (unsigned char)value);
    if (type == 2) return static_cast<Array256_2Bit*>(static_cast<void*>(p))->set((unsigned char)index, (unsigned char)value);
    if (type == 3)
    {
        const unsigned char position = static_cast<unsigned char>(index);
        const unsigned int shift = (position & 1) * 4;
        unsigned char& packed = p[position >> 1];
        packed = static_cast<unsigned char>((packed & ~(15u << shift)) |
                                            (static_cast<unsigned int>(value) << shift));
        return 1;
    }
    return 0;
}

unsigned char SparseDataStorage::PlaneOffsets::getDataSizeFromType(int type) { return type == 1 ? 0x24 : type == 2 ? 0x44 : type == 3 ? 0x80 : 0; }
unsigned char SparseDataStorage::PlaneOffsets::getDataSize(int plane) { return getDataSizeFromType(offsets[plane] >> 14); }
unsigned int SparseDataStorage::PlaneOffsets::getData(int plane, int index)
{
    unsigned short entry = offsets[plane]; unsigned int type = entry >> 14;
    if (!type) return entry & 0x3fff;
    unsigned char* p = (unsigned char*)this + (entry & 0x3fff) * 4;
    if (type == 1) return static_cast<Array256_1Bit*>(static_cast<void*>(p))->get((unsigned char)index);
    if (type == 2) return static_cast<Array256_2Bit*>(static_cast<void*>(p))->get((unsigned char)index);
    unsigned char position = (unsigned char)index;
    return (p[position >> 1] >> ((position & 1) << 2)) & 15;
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
void SparseDataStorage::PlaneOffsets::setDataOffset(int plane, unsigned short offset)
{
    offsets[plane] = offset;
}
unsigned short SparseDataStorage::PlaneOffsets::getDataOffset(int plane)
{
    return offsets[plane] & 0x3fff;
}
unsigned int SparseDataStorage::PlaneOffsets::getData(int plane, char (&values)[256])
{
    for (int i = 0; i < 256; ++i)
    {
        values[i] = (char)getData(plane, i);
    }
    return 256;
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
    return static_cast<long long>((static_cast<unsigned long long>(static_cast<unsigned int>(size)) << 48) |
                                  static_cast<unsigned int>(reinterpret_cast<uintptr_t>(offsets)));
}
SparseDataStorage::SparseDataStorage()
{
    PlaneOffsets* offsets = allocatePlaneOffsets(0x4080);
    unsigned char* payload = reinterpret_cast<unsigned char*>(offsets) + 0x100;
    offsets->offsets[0] = 0;
    for (int plane = 1; plane < 128; ++plane)
    {
        offsets->offsets[plane] = 0xc000;
        offsets->initPlane(plane, payload);
        payload += 0x80;
    }
    XMemSet(reinterpret_cast<unsigned char*>(offsets) + 0x100, 0, 0x3f80);
    dataAndCount = createDataAndCount(offsets, 0x4080);
}
SparseDataStorage::~SparseDataStorage()
{
    PlaneOffsets* offsets = getPlaneOffsets();
    freePlaneOffsets(offsets, getPlaneOffsetsDataSize());
}
SparseDataStorage::PlaneOffsets* SparseDataStorage::getPlaneOffsets()
{
    return reinterpret_cast<PlaneOffsets*>(static_cast<uintptr_t>(static_cast<unsigned int>(dataAndCount)));
}
unsigned int SparseDataStorage::getPlaneOffsetsDataSize()
{
    return static_cast<unsigned long long>(dataAndCount) >> 48;
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
    do
    {
        previous = dataAndCount;
    }
    while (InterlockedCompareExchangeRelease64(&dataAndCount, replacement, previous) != previous);
    queueForDelete(reinterpret_cast<unsigned char*>(static_cast<uintptr_t>(static_cast<unsigned int>(previous))), oldSize);
}
unsigned int SparseDataStorage::get(int x, int y, int z)
{
    return getPlaneOffsets()->getData(y, x * 16 + z);
}
unsigned int SparseDataStorage::tryUpdateDataAndCount(PlaneOffsets* offsets, int size)
{
    PlaneOffsets* oldOffsets = getPlaneOffsets();
    const int oldSize = getPlaneOffsetsDataSize();
    const long long previous = dataAndCount;
    const long long replacement = createDataAndCount(offsets, size);
    if (InterlockedCompareExchangeRelease64(&dataAndCount, replacement, previous) != previous)
        return 0;
    queueForDelete(reinterpret_cast<unsigned char*>(oldOffsets), oldSize);
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
        unsigned char* dataCursor = reinterpret_cast<unsigned char*>(destination) + 0x100;
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
            freePlaneOffsets(reinterpret_cast<PlaneOffsets*>(data), 0);
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
        destination->offsets[plane] = (unsigned short)(0xc000 | ((payload >> 2) & 0x3fff));
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
