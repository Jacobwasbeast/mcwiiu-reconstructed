#include "game/data/ElementStorage.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"

#include <cstdlib>

extern unsigned int TlsAlloc();
extern void* TlsGetValue(unsigned int index);
extern int TlsSetValue(unsigned int index, void* value);

static unsigned int sElementThreadStorageIndex = TlsAlloc();
static unsigned int sMainThreadStorageCount = 0;
static unsigned int sRenderThreadStorageCount = 0;
static OSFastMutex sElementLocks[128];
static OSFastMutex sThreadCountLock;
extern void EnterCriticalSection(void* mutex);
extern void LeaveCriticalSection(void* mutex);
extern void InitializeCriticalSectionAndSpinCount(void* mutex, unsigned long spinCount);

ElementStorage::ThreadStorage::ThreadStorage()
    : mThreadId(0), mReadLocks(0), mHasTempData(false), mBlockReadLock(false), mDataLock(false), mTempData(0)
{
}

ElementStorage::ElementStorage(bool threaded)
    : HierarchicalElementStorage(), mFrameLastUpdated(0), mCriticalSectionIndex(getCriticalSectionIndex(threaded))
{
}

ElementStorage::ElementStorage(bool threaded, ElementStorage* other)
    : HierarchicalElementStorage(other), mFrameLastUpdated(0), mCriticalSectionIndex(getCriticalSectionIndex(threaded))
{
}

ElementStorage::ElementStorage(bool threaded, arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data, unsigned int headroom)
    : HierarchicalElementStorage(), mFrameLastUpdated(0), mCriticalSectionIndex(0)
{
    _Init(blocks, data, headroom);
    mCriticalSectionIndex = getCriticalSectionIndex(threaded);
}

ElementStorage::ElementStorage(bool threaded, arrayWithLength<unsigned short> elements, unsigned int headroom)
    : HierarchicalElementStorage(), mFrameLastUpdated(0), mCriticalSectionIndex(0)
{
    _Init(elements, headroom);
    mCriticalSectionIndex = getCriticalSectionIndex(threaded);
}

ElementStorage::ElementStorage(bool threaded, CompressedBlockStorage* blocks, CompressedBlockStorage* extraBlocks, SparseDataStorage* data, SparseDataStorage* extraData)
    : HierarchicalElementStorage(), mFrameLastUpdated(0), mCriticalSectionIndex(0)
{
    _Init(blocks, extraBlocks, data, extraData);
    mCriticalSectionIndex = getCriticalSectionIndex(threaded);
}

ElementStorage::~ElementStorage()
{
}

int ElementStorage::getCriticalSectionIndex(bool mainThread)
{
    int value = std::rand() % 64;
    return mainThread ? value + 64 : value;
}

void ElementStorage::getWriteLock(int)
{
    EnterCriticalSection(&sElementLocks[mCriticalSectionIndex]);
}

void ElementStorage::releaseWriteLock()
{
    mFrameLastUpdated = (unsigned int)gRenderer->GetFramesSinceBoot();
    LeaveCriticalSection(&sElementLocks[mCriticalSectionIndex]);
}

void ElementStorage::removeCompressionHeadroom()
{
    getWriteLock(1);
    HierarchicalElementStorage::removeCompressionHeadroom();
    releaseWriteLock();
}

bool ElementStorage::shouldLockForRead(ThreadStorage* storage)
{
    if (storage->mHasTempData &&
        ((!storage->mBlockReadLock || sMainThreadStorageCount < 2) && sRenderThreadStorageCount < 2))
        return false;
    return true;
}

void ElementStorage::getReadLock(int reason)
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sElementThreadStorageIndex));
    if (shouldLockForRead(storage))
    {
        ++storage->mReadLocks;
        EnterCriticalSection(&sElementLocks[mCriticalSectionIndex]);
    }
}

void ElementStorage::releaseReadLock()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sElementThreadStorageIndex));
    if (storage->mReadLocks != 0)
    {
        --storage->mReadLocks;
        LeaveCriticalSection(&sElementLocks[mCriticalSectionIndex]);
    }
}

bool ElementStorage::isRenderChunkEmpty(int baseY, int* highestNonEmptyY)
{
    getReadLock(0x65);
    bool result = HierarchicalElementStorage::isRenderChunkEmpty(baseY, highestNonEmptyY);
    releaseReadLock();
    return result;
}

void ElementStorage::setData(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data)
{
    getWriteLock(0);
    HierarchicalElementStorage::setData(blocks, data);
    releaseWriteLock();
}

void ElementStorage::setData(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements)
{
    getWriteLock(0);
    unsigned int count = elements.length;
    for (unsigned int i = 0; i < count; ++i)
    {
        if (extraElements.data[i] == 0x90)
        {
            elements.data[i] = (unsigned short)(elements.data[i] | 0x8000);
            extraElements.data[i] = 0;
        }
    }
    HierarchicalElementStorage::setData(elements);
    releaseWriteLock();
}

void ElementStorage::getData(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data, int minSlice, int maxSlice)
{
    getReadLock(0x66);
    HierarchicalElementStorage::getData(blocks, data, minSlice, maxSlice);
    releaseReadLock();
}

void ElementStorage::getData(arrayWithLength<unsigned short> elements, int minSlice, int maxSlice)
{
    getReadLock(0x67);
    HierarchicalElementStorage::getData(elements, minSlice, maxSlice);
    releaseReadLock();
    int slice;
    for (slice = minSlice; slice < maxSlice; ++slice)
    {
        unsigned int base = (((unsigned int)slice & 0x1f) << 2) | (((unsigned int)slice & 0x20) << 10);
        unsigned int column;
        for (column = 0; column < 256; ++column)
        {
            unsigned int y;
            for (y = 0; y < 4; ++y)
            {
                unsigned int index = base | (column << 7) | y;
                elements.data[index] = (unsigned short)(elements.data[index] & 0x7fff);
            }
        }
    }
}

void ElementStorage::getData(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements, int minSlice, int maxSlice)
{
    getReadLock(0x66);
    HierarchicalElementStorage::getData(elements, extraElements, minSlice, maxSlice);
    releaseReadLock();
    int slice;
    for (slice = minSlice; slice < maxSlice; ++slice)
    {
        unsigned int base = (((unsigned int)slice & 0x1f) << 2) | (((unsigned int)slice & 0x20) << 10);
        unsigned int column;
        for (column = 0; column < 256; ++column)
        {
            unsigned int y;
            for (y = 0; y < 4; ++y)
            {
                unsigned int index = base | (column << 7) | y;
                if ((elements.data[index] & 0x8000) != 0)
                    extraElements.data[index] = 0x90;
                elements.data[index] = (unsigned short)(elements.data[index] & 0x7fff);
            }
        }
    }
}

unsigned int ElementStorage::get(unsigned int x, unsigned int y, unsigned int z, bool* isUniformSection)
{
    getReadLock(0x68);
    unsigned int value = HierarchicalElementStorage::get(x, y, z, isUniformSection);
    releaseReadLock();
    return value & 0x7fff;
}

unsigned int ElementStorage::getExtraElement(unsigned int x, unsigned int y, unsigned int z)
{
    getReadLock(0x69);
    unsigned int value = HierarchicalElementStorage::get(x, y, z, 0);
    if ((value & 0x8000) != 0)
    {
        releaseReadLock();
        return 0x90;
    }
    value = HierarchicalElementStorage::getExtraElement(x, y, z);
    releaseReadLock();
    return value;
}

void ElementStorage::getElementAndExtraElement_NoLock(unsigned int x, unsigned int y, unsigned int z, unsigned short* element, unsigned short* extraElement)
{
    unsigned short value = HierarchicalElementStorage::get(x, y, z, 0);
    if ((value & 0x8000) == 0)
    {
        *element = value;
        *extraElement = HierarchicalElementStorage::getExtraElement(x, y, z);
    }
    else
    {
        *element = value & 0x7fff;
        *extraElement = 0x90;
    }
}

void ElementStorage::getElementAndExtraElement(unsigned int x, unsigned int y, unsigned int z, unsigned short* element, unsigned short* extraElement)
{
    getReadLock(0x69);
    getElementAndExtraElement_NoLock(x, y, z, element, extraElement);
    releaseReadLock();
}

void ElementStorage::getReadLockExternal()
{
    getReadLock(0x71);
}
void ElementStorage::releaseReadLockExternal()
{
    releaseReadLock();
}

void ElementStorage::set(unsigned int x, unsigned int y, unsigned int z, unsigned short value)
{
    getReadLock(0x69);
    unsigned short oldValue = HierarchicalElementStorage::get(x, y, z, 0);
    releaseReadLock();
    getWriteLock(2);
    HierarchicalElementStorage::set(x, y, z, (unsigned short)((oldValue & 0x8000) | value));
    releaseWriteLock();
}

void ElementStorage::setExtraElement(unsigned int x, unsigned int y, unsigned int z, unsigned short value)
{
    getReadLock(0x6a);
    unsigned short oldValue = HierarchicalElementStorage::get(x, y, z, 0);
    releaseReadLock();
    if (value == 0x90)
    {
        getWriteLock(3);
        HierarchicalElementStorage::set(x, y, z, (unsigned short)((oldValue & 0x7fff) | 0x8000));
    }
    else
    {
        getWriteLock(4);
        HierarchicalElementStorage::setExtraElement(x, y, z, value);
        releaseWriteLock();
        getWriteLock(3);
        HierarchicalElementStorage::set(x, y, z, (unsigned short)(oldValue & 0x7fff));
    }
    releaseWriteLock();
}

int ElementStorage::setDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset, void (*changedCallback)(int, int, int, void*, int), void* callbackData, int callbackValue)
{
    getWriteLock(5);
    int result = HierarchicalElementStorage::setDataRegion(data, x0, y0, z0, x1, y1, z1, offset, changedCallback, callbackData, callbackValue);
    releaseWriteLock();
    return result;
}

int ElementStorage::getDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset)
{
    getReadLock(0x6b);
    int result = HierarchicalElementStorage::getDataRegion(data, x0, y0, z0, x1, y1, z1, offset);
    releaseReadLock();
    return result;
}

unsigned int ElementStorage::getFrameLastUpdated()
{
    return mFrameLastUpdated;
}
unsigned int ElementStorage::getAllocatedBytes()
{
    getReadLock(0x6c);
    int value = HierarchicalElementStorage::getAllocatedBytes();
    releaseReadLock();
    return value;
}
unsigned int ElementStorage::getRawData(unsigned char* rawData)
{
    getReadLock(0x6e);
    int value = HierarchicalElementStorage::getRawData(rawData);
    releaseReadLock();
    return value;
}
void ElementStorage::setFromRawData(unsigned char* rawData, unsigned int rawDataBytes)
{
    getWriteLock(6);
    HierarchicalElementStorage::setFromRawData(rawData, rawDataBytes);
    releaseWriteLock();
}
int ElementStorage::getRawDataMaxBytes()
{
    return HierarchicalElementStorage::getRawDataMaxBytes();
}
bool ElementStorage::sectionContainsElementData(int x, int y, int z, unsigned short value, unsigned short mask)
{
    getReadLock(0x70);
    bool result = HierarchicalElementStorage::sectionContainsElementData(x, y, z, value, mask);
    releaseReadLock();
    return result;
}
int ElementStorage::getHighestNonEmptyY()
{
    return HierarchicalElementStorage::getHighestNonEmptyY();
}
void ElementStorage::write(DataOutputStream* out)
{
    getReadLock(0x6f);
    HierarchicalElementStorage::write(out);
    releaseReadLock();
}
void ElementStorage::read(DataInputStream* in)
{
    getWriteLock(7);
    HierarchicalElementStorage::read(in);
    releaseWriteLock();
}

void ElementStorage::CreateNewThreadStorage(eHESThreadId threadId)
{
    ThreadStorage* storage = new ThreadStorage();
    storage->mThreadId = (unsigned int)threadId;
    storage->mBlockReadLock = threadId == (eHESThreadId)1 || threadId == (eHESThreadId)2;
    storage->mDataLock = threadId == (eHESThreadId)8 || threadId == (eHESThreadId)10 ||
        (threadId >= (eHESThreadId)16 && threadId <= (eHESThreadId)18);
    storage->mHasTempData = storage->mBlockReadLock || storage->mDataLock;
    TlsSetValue(sElementThreadStorageIndex, storage);
    if (storage->mHasTempData)
        storage->mTempData = (unsigned char*)std::malloc(getRawDataMaxBytes());
    if (storage->mBlockReadLock)
    {
        EnterCriticalSection(&sThreadCountLock);
        ++sMainThreadStorageCount;
        LeaveCriticalSection(&sThreadCountLock);
    }
    if (storage->mDataLock)
    {
        EnterCriticalSection(&sThreadCountLock);
        ++sRenderThreadStorageCount;
        LeaveCriticalSection(&sThreadCountLock);
    }
}

void ElementStorage::ReleaseThreadStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sElementThreadStorageIndex));
    if (storage->mBlockReadLock)
    {
        EnterCriticalSection(&sThreadCountLock);
        --sMainThreadStorageCount;
        LeaveCriticalSection(&sThreadCountLock);
    }
    if (storage->mDataLock)
    {
        EnterCriticalSection(&sThreadCountLock);
        --sRenderThreadStorageCount;
        LeaveCriticalSection(&sThreadCountLock);
    }
    std::free(storage->mTempData);
    delete storage;
}

void ElementStorage::staticCtor()
{
    int i;
    for (i = 0; i < 128; ++i)
        InitializeCriticalSectionAndSpinCount(&sElementLocks[i], 0x1400);
    InitializeCriticalSectionAndSpinCount(&sThreadCountLock, 0x1400);
}
void ElementStorage::getReadLockUnconditional(int)
{
    EnterCriticalSection(&sElementLocks[mCriticalSectionIndex]);
}
void ElementStorage::releaseReadLockUnconditional()
{
    LeaveCriticalSection(&sElementLocks[mCriticalSectionIndex]);
}
unsigned char* ElementStorage::getTempData()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sElementThreadStorageIndex));
    if (storage != 0 && storage->mTempData != 0)
        return storage->mTempData;
    return HierarchicalElementStorage::getTempData();
}
unsigned char* ElementStorage::getAllocatedRawData()
{
    getReadLock(0x6d);
    unsigned char* value = HierarchicalElementStorage::getAllocatedRawData();
    releaseReadLock();
    return value;
}
