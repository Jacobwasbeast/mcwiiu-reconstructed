#ifndef GAME_DATA_ELEMENTSTORAGE_HPP
#define GAME_DATA_ELEMENTSTORAGE_HPP

#include "game/data/HierarchicalElementStorage.hpp"

class CompressedBlockStorage;
class DataInputStream;
class DataOutputStream;
class SparseDataStorage;

class ElementStorage : public HierarchicalElementStorage
{
public:
    enum eHESThreadId
    {
        eHESThreadId_Server = 1,
        eHESThreadId_ServerPostUpdate = 2,
        eHESThreadId_ServerLevel = 7,
        eHESThreadId_NetworkGame = 8,
        eHESThreadId_Main = 10,
        eHESThreadId_Renderer = 11,
        eHESThreadId_GameModeTransition = 16,
        eHESThreadId_RemoteSave = 17,
        eHESThreadId_ClientChunkStorage = 18
    };

    class ThreadStorage
    {
    public:
        eHESThreadId mThreadId;
        int mReadLockCount;
        bool mUsesSharedLocks;
        bool mIsServerThread;
        bool mIsClientThread;
        unsigned char* mTempData;
    };

    ElementStorage(bool isClientSide);
    ElementStorage(bool isClientSide, ElementStorage* other);
    ElementStorage(bool isClientSide, arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data, unsigned int headroom);
    ElementStorage(bool isClientSide, arrayWithLength<unsigned short> elements, unsigned int headroom);
    ElementStorage(bool isClientSide, CompressedBlockStorage* blocks, CompressedBlockStorage* extraBlocks, SparseDataStorage* data, SparseDataStorage* extraData);

    void setData(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data);
    void setData(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements);

    void getData(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data, int minSlice, int maxSlice);
    void getData(arrayWithLength<unsigned short> elements, int minSlice, int maxSlice);
    void getData(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements, int minSlice, int maxSlice);

    unsigned short get(unsigned int x, unsigned int y, unsigned int z, bool* isUniformSection);
    unsigned short getExtraElement(unsigned int x, unsigned int y, unsigned int z);

    void getElementAndExtraElement_NoLock(unsigned int x, unsigned int y, unsigned int z, unsigned short* element, unsigned short* extraElement);
    void getElementAndExtraElement(unsigned int x, unsigned int y, unsigned int z, unsigned short* element, unsigned short* extraElement);

    void set(unsigned int x, unsigned int y, unsigned int z, unsigned short value);
    void setExtraElement(unsigned int x, unsigned int y, unsigned int z, unsigned short value);

    int setDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset, void (*changedCallback)(int, int, int, void*, int), void* callbackData, int callbackValue);
    int getDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset);

    bool isRenderChunkEmpty(int baseY, int* highestNonEmptyY);
    int getHighestNonEmptyY();

    bool sectionContainsElementData(int x, int y, int z, unsigned short value, unsigned short mask);

    void removeCompressionHeadroom();

    int getFrameLastUpdated();

    int getAllocatedBytes();
    unsigned char* getAllocatedRawData();

    int getRawData(unsigned char* rawData);
    void setFromRawData(unsigned char* rawData, unsigned int rawDataBytes);
    static int getRawDataMaxBytes();

    void write(DataOutputStream* out);
    void read(DataInputStream* in);

    void getReadLockExternal();
    void releaseReadLockExternal();

    static void CreateNewThreadStorage(eHESThreadId threadId);
    static void ReleaseThreadStorage();
    static void staticCtor();

    virtual void getReadLockUnconditional(int reason);
    virtual void releaseReadLockUnconditional();
    virtual unsigned char* getTempData();

    virtual ~ElementStorage();

private:
    int getCriticalSectionIndex(bool isClientSide);

    bool shouldLockForRead(ThreadStorage* threadStorage);

    void getWriteLock(int reason);
    void releaseWriteLock();

    void getReadLock(int reason);
    void releaseReadLock();

    int mFrameLastUpdated;
    int mCriticalSectionIndex;
};

#endif
