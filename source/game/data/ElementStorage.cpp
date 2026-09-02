#include "game/data/ElementStorage.hpp"

#include <cstdlib>

#include "game/client/render/Renderer.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"

unsigned int TlsAlloc();
void TlsSetValue(unsigned int index, void* value);
void* TlsGetValue(unsigned int index);

static const int CRITICAL_SECTIONS_PER_SIDE = 64;
static const int CRITICAL_SECTION_COUNT = CRITICAL_SECTIONS_PER_SIDE * 2;
static const unsigned long CRITICAL_SECTION_SPIN_COUNT = 0x1400;

static const unsigned short EXTRA_ELEMENT_DEFAULT = 0x90;
static const unsigned short EXTRA_ELEMENT_DEFAULT_FLAG = 0x8000;
static const unsigned short ELEMENT_MASK = 0x7FFF;

static const unsigned int SLICE_COLUMN_COUNT = 256;
static const unsigned int SLICE_COLUMN_HEIGHT = 4;
static const unsigned int SLICE_COLUMN_SHIFT = 7;

static const int WRITE_LOCK_SET_DATA = 0;
static const int WRITE_LOCK_REMOVE_COMPRESSION_HEADROOM = 1;
static const int WRITE_LOCK_SET = 2;
static const int WRITE_LOCK_SET_ELEMENT = 3;
static const int WRITE_LOCK_SET_EXTRA_ELEMENT = 4;
static const int WRITE_LOCK_SET_DATA_REGION = 5;
static const int WRITE_LOCK_SET_FROM_RAW_DATA = 6;
static const int WRITE_LOCK_READ = 7;

static const int READ_LOCK_IS_RENDER_CHUNK_EMPTY = 101;
static const int READ_LOCK_GET_DATA = 102;
static const int READ_LOCK_GET_ELEMENT_RANGE = 103;
static const int READ_LOCK_GET = 104;
static const int READ_LOCK_GET_EXTRA_ELEMENT = 105;
static const int READ_LOCK_SET_EXTRA_ELEMENT = 106;
static const int READ_LOCK_GET_DATA_REGION = 107;
static const int READ_LOCK_GET_ALLOCATED_BYTES = 108;
static const int READ_LOCK_GET_RAW_DATA = 110;
static const int READ_LOCK_WRITE = 111;
static const int READ_LOCK_SECTION_CONTAINS_ELEMENT_DATA = 112;
static const int READ_LOCK_EXTERNAL = 113;

static OSFastMutex sElementLocks[CRITICAL_SECTION_COUNT];
static OSFastMutex sThreadCountLock;

static unsigned int sTlsIndex = TlsAlloc();

static int sServerThreadCount = 0;
static int sClientThreadCount = 0;

int ElementStorage::getCriticalSectionIndex(bool isClientSide)
{
    int index = rand() % CRITICAL_SECTIONS_PER_SIDE;

    if (isClientSide)
    {
        index += CRITICAL_SECTIONS_PER_SIDE;
    }

    return index;
}

ElementStorage::ElementStorage(bool isClientSide)
    : HierarchicalElementStorage()
{
    mFrameLastUpdated = 0;
    mCriticalSectionIndex = getCriticalSectionIndex(isClientSide);
}

ElementStorage::ElementStorage(bool isClientSide, ElementStorage* other)
    : HierarchicalElementStorage(other)
{
    mFrameLastUpdated = 0;
    mCriticalSectionIndex = getCriticalSectionIndex(isClientSide);
}

ElementStorage::ElementStorage(bool isClientSide, arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data, unsigned int headroom)
    : HierarchicalElementStorage()
{
    _Init(blocks, data, headroom);

    mFrameLastUpdated = 0;
    mCriticalSectionIndex = getCriticalSectionIndex(isClientSide);
}

ElementStorage::ElementStorage(bool isClientSide, arrayWithLength<unsigned short> elements, unsigned int headroom)
    : HierarchicalElementStorage()
{
    _Init(elements, headroom);

    mFrameLastUpdated = 0;
    mCriticalSectionIndex = getCriticalSectionIndex(isClientSide);
}

ElementStorage::ElementStorage(bool isClientSide, CompressedBlockStorage* blocks, CompressedBlockStorage* extraBlocks, SparseDataStorage* data, SparseDataStorage* extraData)
    : HierarchicalElementStorage()
{
    _Init(blocks, extraBlocks, data, extraData);

    mFrameLastUpdated = 0;
    mCriticalSectionIndex = getCriticalSectionIndex(isClientSide);
}

ElementStorage::~ElementStorage()
{
}

void ElementStorage::getWriteLock(int reason)
{
    ThreadStorage* threadStorage = (ThreadStorage*)TlsGetValue(sTlsIndex);

    EnterCriticalSection(&sElementLocks[mCriticalSectionIndex]);
}

void ElementStorage::releaseWriteLock()
{
    mFrameLastUpdated = gRenderer->GetFramesSinceBoot();

    LeaveCriticalSection(&sElementLocks[mCriticalSectionIndex]);
}

void ElementStorage::removeCompressionHeadroom()
{
    getWriteLock(WRITE_LOCK_REMOVE_COMPRESSION_HEADROOM);

    HierarchicalElementStorage::removeCompressionHeadroom();

    releaseWriteLock();
}

bool ElementStorage::shouldLockForRead(ThreadStorage* threadStorage)
{
    if (threadStorage->mUsesSharedLocks)
    {
        if (threadStorage->mIsServerThread && sServerThreadCount >= 2)
        {
            return true;
        }

        if (sClientThreadCount >= 2)
        {
            return true;
        }

        return false;
    }

    return true;
}

void ElementStorage::getReadLock(int reason)
{
    ThreadStorage* threadStorage = (ThreadStorage*)TlsGetValue(sTlsIndex);

    if (shouldLockForRead(threadStorage))
    {
        ++threadStorage->mReadLockCount;

        EnterCriticalSection(&sElementLocks[mCriticalSectionIndex]);
    }
}

void ElementStorage::releaseReadLock()
{
    ThreadStorage* threadStorage = (ThreadStorage*)TlsGetValue(sTlsIndex);

    if (threadStorage->mReadLockCount > 0)
    {
        --threadStorage->mReadLockCount;

        LeaveCriticalSection(&sElementLocks[mCriticalSectionIndex]);
    }
}

bool ElementStorage::isRenderChunkEmpty(int baseY, int* highestNonEmptyY)
{
    getReadLock(READ_LOCK_IS_RENDER_CHUNK_EMPTY);

    bool empty = HierarchicalElementStorage::isRenderChunkEmpty(baseY, highestNonEmptyY);

    releaseReadLock();

    return empty;
}

void ElementStorage::setData(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data)
{
    getWriteLock(WRITE_LOCK_SET_DATA);

    HierarchicalElementStorage::setData(blocks, data);

    releaseWriteLock();
}

void ElementStorage::setData(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements)
{
    getWriteLock(WRITE_LOCK_SET_DATA);

    for (unsigned int index = 0; index < elements.length; ++index)
    {
        if (extraElements.data[index] == EXTRA_ELEMENT_DEFAULT)
        {
            elements.data[index] = (unsigned short)(elements.data[index] | EXTRA_ELEMENT_DEFAULT_FLAG);
            extraElements.data[index] = 0;
        }
    }

    HierarchicalElementStorage::setData(elements);

    releaseWriteLock();
}

void ElementStorage::getData(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data, int minSlice, int maxSlice)
{
    getReadLock(READ_LOCK_GET_DATA);

    HierarchicalElementStorage::getData(blocks, data, minSlice, maxSlice);

    releaseReadLock();
}

void ElementStorage::getData(arrayWithLength<unsigned short> elements, int minSlice, int maxSlice)
{
    getReadLock(READ_LOCK_GET_ELEMENT_RANGE);

    HierarchicalElementStorage::getData(elements, minSlice, maxSlice);

    releaseReadLock();

    for (int slice = minSlice; slice < maxSlice; ++slice)
    {
        unsigned int sliceBaseIndex = (((unsigned int)slice & 0x1F) << 2) | (((unsigned int)slice & 0x20) << 10);

        for (unsigned int column = 0; column < SLICE_COLUMN_COUNT; ++column)
        {
            for (unsigned int y = 0; y < SLICE_COLUMN_HEIGHT; ++y)
            {
                unsigned int elementIndex = sliceBaseIndex | (column << SLICE_COLUMN_SHIFT) | y;

                elements.data[elementIndex] = (unsigned short)(elements.data[elementIndex] & ELEMENT_MASK);
            }
        }
    }
}

void ElementStorage::getData(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements, int minSlice, int maxSlice)
{
    getReadLock(READ_LOCK_GET_DATA);

    HierarchicalElementStorage::getData(elements, extraElements, minSlice, maxSlice);

    releaseReadLock();

    for (int slice = minSlice; slice < maxSlice; ++slice)
    {
        unsigned int sliceBaseIndex = (((unsigned int)slice & 0x1F) << 2) | (((unsigned int)slice & 0x20) << 10);

        for (unsigned int column = 0; column < SLICE_COLUMN_COUNT; ++column)
        {
            for (unsigned int y = 0; y < SLICE_COLUMN_HEIGHT; ++y)
            {
                unsigned int elementIndex = sliceBaseIndex | (column << SLICE_COLUMN_SHIFT) | y;

                if ((elements.data[elementIndex] & EXTRA_ELEMENT_DEFAULT_FLAG) != 0)
                {
                    extraElements.data[elementIndex] = EXTRA_ELEMENT_DEFAULT;
                }

                elements.data[elementIndex] = (unsigned short)(elements.data[elementIndex] & ELEMENT_MASK);
            }
        }
    }
}

unsigned short ElementStorage::get(unsigned int x, unsigned int y, unsigned int z, bool* isUniformSection)
{
    getReadLock(READ_LOCK_GET);

    unsigned short element = HierarchicalElementStorage::get(x, y, z, isUniformSection);

    releaseReadLock();

    return (unsigned short)(element & ELEMENT_MASK);
}

unsigned short ElementStorage::getExtraElement(unsigned int x, unsigned int y, unsigned int z)
{
    getReadLock(READ_LOCK_GET_EXTRA_ELEMENT);

    if ((HierarchicalElementStorage::get(x, y, z, NULL) & EXTRA_ELEMENT_DEFAULT_FLAG) != 0)
    {
        releaseReadLock();

        return EXTRA_ELEMENT_DEFAULT;
    }

    unsigned short extraElement = HierarchicalElementStorage::getExtraElement(x, y, z);

    releaseReadLock();

    return extraElement;
}

void ElementStorage::getElementAndExtraElement_NoLock(unsigned int x, unsigned int y, unsigned int z, unsigned short* element, unsigned short* extraElement)
{
    unsigned short value = HierarchicalElementStorage::get(x, y, z, NULL);

    if ((value & EXTRA_ELEMENT_DEFAULT_FLAG) != 0)
    {
        *element = (unsigned short)(value & ELEMENT_MASK);
        *extraElement = EXTRA_ELEMENT_DEFAULT;
    }
    else
    {
        *extraElement = HierarchicalElementStorage::getExtraElement(x, y, z);
        *element = value;
    }
}

void ElementStorage::getElementAndExtraElement(unsigned int x, unsigned int y, unsigned int z, unsigned short* element, unsigned short* extraElement)
{
    getReadLock(READ_LOCK_GET_EXTRA_ELEMENT);

    getElementAndExtraElement_NoLock(x, y, z, element, extraElement);

    releaseReadLock();
}

void ElementStorage::getReadLockExternal()
{
    getReadLock(READ_LOCK_EXTERNAL);
}

void ElementStorage::releaseReadLockExternal()
{
    releaseReadLock();
}

void ElementStorage::set(unsigned int x, unsigned int y, unsigned int z, unsigned short value)
{
    getReadLock(READ_LOCK_GET_EXTRA_ELEMENT);

    unsigned short element = HierarchicalElementStorage::get(x, y, z, NULL);

    releaseReadLock();

    getWriteLock(WRITE_LOCK_SET);

    HierarchicalElementStorage::set(x, y, z, (unsigned short)((element & EXTRA_ELEMENT_DEFAULT_FLAG) | value));

    releaseWriteLock();
}

void ElementStorage::setExtraElement(unsigned int x, unsigned int y, unsigned int z, unsigned short value)
{
    getReadLock(READ_LOCK_SET_EXTRA_ELEMENT);

    unsigned short element = HierarchicalElementStorage::get(x, y, z, NULL);

    releaseReadLock();

    if (value == EXTRA_ELEMENT_DEFAULT)
    {
        getWriteLock(WRITE_LOCK_SET_ELEMENT);

        HierarchicalElementStorage::set(x, y, z, (unsigned short)((element & ELEMENT_MASK) | EXTRA_ELEMENT_DEFAULT_FLAG));

        releaseWriteLock();
    }
    else
    {
        getWriteLock(WRITE_LOCK_SET_EXTRA_ELEMENT);

        HierarchicalElementStorage::setExtraElement(x, y, z, value);

        releaseWriteLock();

        getWriteLock(WRITE_LOCK_SET_ELEMENT);

        HierarchicalElementStorage::set(x, y, z, (unsigned short)(element & ELEMENT_MASK));

        releaseWriteLock();
    }
}

int ElementStorage::setDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset, void (*changedCallback)(int, int, int, void*, int), void* callbackData, int callbackValue)
{
    getWriteLock(WRITE_LOCK_SET_DATA_REGION);

    int changedCount = HierarchicalElementStorage::setDataRegion(data, x0, y0, z0, x1, y1, z1, offset, changedCallback, callbackData, callbackValue);

    releaseWriteLock();

    return changedCount;
}

int ElementStorage::getDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset)
{
    getReadLock(READ_LOCK_GET_DATA_REGION);

    int copiedCount = HierarchicalElementStorage::getDataRegion(data, x0, y0, z0, x1, y1, z1, offset);

    releaseReadLock();

    return copiedCount;
}

int ElementStorage::getFrameLastUpdated()
{
    return mFrameLastUpdated;
}

int ElementStorage::getAllocatedBytes()
{
    getReadLock(READ_LOCK_GET_ALLOCATED_BYTES);

    int allocatedBytes = HierarchicalElementStorage::getAllocatedBytes();

    releaseReadLock();

    return allocatedBytes;
}

int ElementStorage::getRawData(unsigned char* rawData)
{
    getReadLock(READ_LOCK_GET_RAW_DATA);

    int rawDataBytes = HierarchicalElementStorage::getRawData(rawData);

    releaseReadLock();

    return rawDataBytes;
}

void ElementStorage::setFromRawData(unsigned char* rawData, unsigned int rawDataBytes)
{
    getWriteLock(WRITE_LOCK_SET_FROM_RAW_DATA);

    HierarchicalElementStorage::setFromRawData(rawData, rawDataBytes);

    releaseWriteLock();
}

int ElementStorage::getRawDataMaxBytes()
{
    return HierarchicalElementStorage::getRawDataMaxBytes();
}

bool ElementStorage::sectionContainsElementData(int x, int y, int z, unsigned short value, unsigned short mask)
{
    getReadLock(READ_LOCK_SECTION_CONTAINS_ELEMENT_DATA);

    bool contains = HierarchicalElementStorage::sectionContainsElementData(x, y, z, value, mask);

    releaseReadLock();

    return contains;
}

int ElementStorage::getHighestNonEmptyY()
{
    return HierarchicalElementStorage::getHighestNonEmptyY();
}

void ElementStorage::write(DataOutputStream* out)
{
    getReadLock(READ_LOCK_WRITE);

    HierarchicalElementStorage::write(out);

    releaseReadLock();
}

void ElementStorage::read(DataInputStream* in)
{
    getWriteLock(WRITE_LOCK_READ);

    HierarchicalElementStorage::read(in);

    releaseWriteLock();
}

void ElementStorage::CreateNewThreadStorage(eHESThreadId threadId)
{
    ThreadStorage* threadStorage = new ThreadStorage();

    threadStorage->mThreadId = threadId;
    threadStorage->mReadLockCount = 0;

    threadStorage->mIsServerThread = (threadId == eHESThreadId_Server || threadId == eHESThreadId_ServerPostUpdate);
    threadStorage->mIsClientThread = (threadId == eHESThreadId_NetworkGame || threadId == eHESThreadId_Main ||
                                      (threadId >= eHESThreadId_GameModeTransition && threadId <= eHESThreadId_ClientChunkStorage));

    threadStorage->mUsesSharedLocks = (threadStorage->mIsServerThread || threadStorage->mIsClientThread);

    TlsSetValue(sTlsIndex, threadStorage);

    threadStorage->mTempData = threadStorage->mUsesSharedLocks ? (unsigned char*)malloc(getRawDataMaxBytes()) : NULL;

    if (threadStorage->mIsServerThread)
    {
        EnterCriticalSection(&sThreadCountLock);
        ++sServerThreadCount;
        LeaveCriticalSection(&sThreadCountLock);
    }

    if (threadStorage->mIsClientThread)
    {
        EnterCriticalSection(&sThreadCountLock);
        ++sClientThreadCount;
        LeaveCriticalSection(&sThreadCountLock);
    }
}

void ElementStorage::ReleaseThreadStorage()
{
    ThreadStorage* threadStorage = (ThreadStorage*)TlsGetValue(sTlsIndex);

    if (threadStorage->mIsServerThread)
    {
        EnterCriticalSection(&sThreadCountLock);
        --sServerThreadCount;
        LeaveCriticalSection(&sThreadCountLock);
    }

    if (threadStorage->mIsClientThread)
    {
        EnterCriticalSection(&sThreadCountLock);
        --sClientThreadCount;
        LeaveCriticalSection(&sThreadCountLock);
    }

    free(threadStorage->mTempData);

    delete threadStorage;
}

void ElementStorage::staticCtor()
{
    for (int index = 0; index < CRITICAL_SECTION_COUNT; ++index)
    {
        InitializeCriticalSectionAndSpinCount(&sElementLocks[index], CRITICAL_SECTION_SPIN_COUNT);
    }

    InitializeCriticalSectionAndSpinCount(&sThreadCountLock, CRITICAL_SECTION_SPIN_COUNT);
}

void ElementStorage::getReadLockUnconditional(int reason)
{
    EnterCriticalSection(&sElementLocks[mCriticalSectionIndex]);
}

void ElementStorage::releaseReadLockUnconditional()
{
    LeaveCriticalSection(&sElementLocks[mCriticalSectionIndex]);
}

unsigned char* ElementStorage::getTempData()
{
    ThreadStorage* threadStorage = (ThreadStorage*)TlsGetValue(sTlsIndex);

    if (threadStorage != NULL && threadStorage->mTempData != NULL)
    {
        return threadStorage->mTempData;
    }

    return HierarchicalElementStorage::getTempData();
}
