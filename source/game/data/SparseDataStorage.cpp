#include "game/data/SparseDataStorage.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/XLockFreeStack.hpp"
#include <string.h>
#include <vector>

static int sPendingDeleteList;
static XLockFreeStack<unsigned char> sPendingDeleteLists[3];

void SparseDataStorage::Array256_1Bit::init()
{
    bytes[32] = 255; bytes[33] = 255; bytes[34] = 0; bytes[35] = 0;
}

unsigned int SparseDataStorage::Array256_1Bit::set(unsigned char index, unsigned char value)
{
    unsigned int p = index >> 3; unsigned char mask = (unsigned char)(1 << (index & 7));
    if (value == bytes[32]) { bytes[p] &= (unsigned char)~mask; return 1; }
    if (bytes[32] == 255) { bytes[32] = value; bytes[p] &= (unsigned char)~mask; return 1; }
    if (value == bytes[33]) { bytes[p] |= mask; return 1; }
    if (bytes[33] == 255) { bytes[33] = value; bytes[p] |= mask; return 1; }
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
    unsigned int p = index >> 2; unsigned int shift = (index & 3) << 1; unsigned char mask = (unsigned char)(3 << shift);
    unsigned char* slot = &bytes[p];
    if (value == bytes[64]) { *slot &= (unsigned char)~mask; return 1; }
    if (bytes[64] == 255) { bytes[64] = value; *slot &= (unsigned char)~mask; return 1; }
    if (value == bytes[65]) { *slot = (unsigned char)((*slot & ~mask) | (1 << shift)); return 1; }
    if (bytes[65] == 255) { bytes[65] = value; *slot = (unsigned char)((*slot & ~mask) | (1 << shift)); return 1; }
    if (value == bytes[66]) { *slot = (unsigned char)((*slot & ~mask) | (2 << shift)); return 1; }
    if (bytes[66] == 255) { bytes[66] = value; *slot = (unsigned char)((*slot & ~mask) | (2 << shift)); return 1; }
    if (value == bytes[67]) { *slot |= mask; return 1; }
    if (bytes[67] == 255) { bytes[67] = value; *slot |= mask; return 1; }
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
    if (type == 3) { p[index >> 1] = (unsigned char)((p[index >> 1] & ~(15 << ((index & 1) << 2))) | ((value & 15) << ((index & 1) << 2))); return 1; }
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
    return (p[index >> 1] >> ((index & 1) << 2)) & 15;
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
        if (!found && count < 16)
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
    return static_cast<PlaneOffsets*>(static_cast<void*>(new unsigned char[size]));
}
void SparseDataStorage::freePlaneOffsets(PlaneOffsets* offsets, int)
{
    delete[] static_cast<unsigned char*>(static_cast<void*>(offsets));
}
int SparseDataStorage::createDataAndCount(PlaneOffsets* offsets, int size)
{
    planeOffsets = offsets;
    dataAndCount = (unsigned int)size << 16;
    return dataAndCount;
}
SparseDataStorage::SparseDataStorage() : dataAndCount(0), planeOffsets(NULL)
{
    planeOffsets = allocatePlaneOffsets(0x4080);
    memset(planeOffsets, 0, 0x4080);
    dataAndCount = 0x40800000;
}
SparseDataStorage::SparseDataStorage(bool) : dataAndCount(0), planeOffsets(NULL)
{
    planeOffsets = allocatePlaneOffsets(0x4080);
    memset(planeOffsets, 0, 0x4080);
    dataAndCount = 0x40800000;
}
SparseDataStorage::SparseDataStorage(SparseDataStorage* other)
    : dataAndCount(other->dataAndCount), planeOffsets(allocatePlaneOffsets(other->getPlaneOffsetsDataSize()))
{
    memcpy(planeOffsets, other->planeOffsets, other->getPlaneOffsetsDataSize());
}
SparseDataStorage::~SparseDataStorage()
{
    freePlaneOffsets(planeOffsets, getPlaneOffsetsDataSize());
}
SparseDataStorage::PlaneOffsets* SparseDataStorage::getPlaneOffsets()
{
    return planeOffsets;
}
unsigned int SparseDataStorage::getPlaneOffsetsDataSize()
{
    return dataAndCount >> 16;
}
void SparseDataStorage::queueForDelete(unsigned char* data, int)
{
    if (data != NULL)
        sPendingDeleteLists[sPendingDeleteList].Push(data);
}
void SparseDataStorage::updateDataAndCount(PlaneOffsets* p, int size)
{
    PlaneOffsets* old = planeOffsets;
    planeOffsets = p;
    dataAndCount = (unsigned int)size << 16;
    queueForDelete((unsigned char*)old, size);
}
uint64_t SparseDataStorage::get(int x, int y, int z)
{
    return (uint64_t)planeOffsets->getData(y, x * 16 + z) << 32;
}
unsigned int SparseDataStorage::tryUpdateDataAndCount(PlaneOffsets* p, int size)
{
    if (planeOffsets != p)
    {
        updateDataAndCount(p, size);
        return 1;
    }
    return 0;
}
void SparseDataStorage::staticCtor() {}
unsigned int SparseDataStorage::compress() { return getPlaneOffsetsDataSize(); }
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
    unsigned int nonEmpty = 0;
    for (int plane = 0; plane < 128; ++plane)
        if (planeOffsets->offsets[plane] != 0)
            ++nonEmpty;

    const unsigned int blockSize = 0x80;
    const unsigned int totalSize = blockSize * (nonEmpty + 1);
    std::vector<unsigned char> encoded(totalSize, 0);
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
                unsigned int low = planeOffsets->getData(plane, first + pair * 2) & 15;
                unsigned int high = planeOffsets->getData(plane, first + pair * 2 + 1) & 15;
                destination[group * 8 + pair] = (unsigned char)(low | (high << 4));
            }
        }
    }
    stream->writeInt((int)nonEmpty);
    arrayWithLength<unsigned char> output(totalSize, false);
    memcpy(output.data, &encoded[0], totalSize);
    stream->write(output);
}
void SparseDataStorage::read(DataInputStream* stream)
{
    const unsigned int nonEmpty = (unsigned int)stream->readInt();
    const unsigned int inputSize = 0x80 * (nonEmpty + 1);
    std::vector<unsigned char> encoded(inputSize);
    arrayWithLength<unsigned char> input(inputSize, false);
    stream->readFully(input);
    memcpy(&encoded[0], input.data, inputSize);

    const unsigned int requiredSize = nonEmpty * 0x80 + 0x100;
    PlaneOffsets* destination = planeOffsets;
    bool replaced = false;
    if (getPlaneOffsetsDataSize() != requiredSize)
    {
        destination = allocatePlaneOffsets(requiredSize);
        memset(destination, 0, requiredSize);
        replaced = true;
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
}
bool SparseDataStorage::isSameAs(SparseDataStorage* other)
{
    return other && dataAndCount == other->dataAndCount &&
        memcmp(planeOffsets, other->planeOffsets, getPlaneOffsetsDataSize()) == 0;
}
unsigned int SparseDataStorage::getValuesInPlane(arrayWithLength<unsigned char> values, unsigned int plane, int, char (&unique)[16])
{
    return planeOffsets->getValuesInPlane((int)plane, unique);
}
void SparseDataStorage::setData(arrayWithLength<unsigned char> values, unsigned int plane, bool)
{
    for (unsigned int i = 0; i < values.length && i < 256; ++i)
    {
        planeOffsets->setData((int)plane, (int)i, values.data[i]);
    }
}
void SparseDataStorage::getData(arrayWithLength<unsigned char> values, unsigned int plane, int offset, int count)
{
    for (int i = 0; i < count && (unsigned int)i < values.length; ++i)
    {
        values.data[i] = (unsigned char)planeOffsets->getData((int)plane, offset + i);
    }
}
void SparseDataStorage::increasePlaneBitdepth(int plane)
{
    PlaneOffsets* oldOffsets = getPlaneOffsets();
    unsigned int oldSize = getPlaneOffsetsDataSize();
    unsigned int newSize = oldSize + 0x80;
    PlaneOffsets* newOffsets = allocatePlaneOffsets(newSize);
    memcpy(newOffsets, oldOffsets, oldSize);

    unsigned char* packed = (unsigned char*)newOffsets + oldSize;
    for (int index = 0; index < 256; ++index)
    {
        unsigned int value = oldOffsets->getData(plane, index) & 0xf;
        unsigned int byteIndex = (unsigned int)(index >> 1);
        unsigned int shift = (unsigned int)((index & 1) << 2);
        packed[byteIndex] = (unsigned char)((packed[byteIndex] & (unsigned char)~(0xf << shift)) | (unsigned char)(value << shift));
    }
    newOffsets->setDataOffset(plane, (unsigned short)(((oldSize >> 2) & 0x3fff) | 0xc000));
    updateDataAndCount(newOffsets, (int)newSize);
}
void SparseDataStorage::set(int x, int y, int z, int value) { planeOffsets->setData(y, x * 16 + z, value); }
bool SparseDataStorage::testSetDataRegion(arrayWithLength<unsigned char> values, int x, int y, int z, int width, int height, int depth, int value, bool* changed)
{
    if (changed)
    {
        *changed = false;
    }
    setDataRegion(values, x, y, z, width, height, depth, value, NULL, NULL);
    return true;
}
void SparseDataStorage::setDataRegion(arrayWithLength<unsigned char> values, int x, int y, int z, int width, int height, int depth, int value, void*, void*)
{
    for (int yy = 0; yy < height; ++yy)
    {
        for (int zz = 0; zz < depth; ++zz)
        {
            for (int xx = 0; xx < width; ++xx)
            {
                set(x + xx, y + yy, z + zz, value);
            }
        }
    }
}
void SparseDataStorage::getDataRegion(arrayWithLength<unsigned char> values, int x, int y, int z, int width, int height, int depth, int)
{
    unsigned int n = 0;
    for (int yy = 0; yy < height; ++yy)
    {
        for (int zz = 0; zz < depth; ++zz)
        {
            for (int xx = 0; xx < width && n < values.length; ++xx)
            {
                values.data[n++] = (unsigned char)get(x + xx, y + yy, z + zz);
            }
        }
    }
}
bool SparseDataStorage::isCompressed() { return getPlaneOffsetsDataSize() < 0x4080; }
