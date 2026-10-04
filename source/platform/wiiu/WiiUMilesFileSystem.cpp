#include "platform/wiiu/WiiUMilesFileSystem.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"
#include "game/core/CriticalSection.hpp"
#include <vector>
#include <cstdlib>
#include <cstring>

struct _OVERLAPPED;
extern unsigned int ReadFile(int hFile, void* lpBuffer, unsigned int nNumberOfBytesToRead, unsigned int* lpNumberOfBytesRead, _OVERLAPPED* lpOverlapped);

struct CFileHandle
{
    char mFilename[0x104];
    int mFileHandle;
    int mStartOffset;
    int mLength;
    int mCurrentPos;
    unsigned char mValidPosition;
    char mPadding[3];
    int mFileSize;
};

static unsigned char sInitialized = 0;
static std::vector<CFileHandle*> sFileHandles;
static OSFastMutex sMutex;

void InvalidePositionsForOtherHandles(CFileHandle* handle)
{
    EnterCriticalSection(&sMutex);
    for (size_t i = 0; i < sFileHandles.size(); ++i)
    {
        CFileHandle* other = sFileHandles[i];
        if (other != handle && other->mFileHandle == handle->mFileHandle)
        {
            other->mValidPosition = 0;
        }
    }
    LeaveCriticalSection(&sMutex);
}

unsigned int file_open_callback(const char* filename, unsigned int* handleOut)
{
    if (sInitialized == 0)
    {
        InitializeCriticalSection(&sMutex);
        sInitialized = 1;
    }

    if (filename[0] == '*')
    {
        char localName[260];
        const char* p = filename + 1;
        char* dst = localName;
        while (*p != '*')
        {
            *dst++ = *p++;
        }
        *dst = '\0';
        p++;

        CFileHandle* handle = NULL;
        for (size_t i = 0; i < sFileHandles.size(); ++i)
        {
            if (strcmp(sFileHandles[i]->mFilename, localName) == 0)
            {
                CFileHandle* existingHandle = sFileHandles[i];
                handle = new CFileHandle;
                if (handle != NULL)
                {
                    handle->mFileSize = existingHandle->mFileSize;
                    handle->mStartOffset = 0;
                    handle->mCurrentPos = 0;
                    handle->mValidPosition = 1;
                    handle->mFileHandle = existingHandle->mFileHandle;
                    handle->mLength = 0;
                    strncpy(handle->mFilename, filename, 0x104);
                }
                break;
            }
        }

        if (handle == NULL)
        {
            int fd = CreateFileA(localName, 0x80000000, 0, NULL, 3, 0, 0);
            int size = SetFilePointer(fd, 0, NULL, 2);
            SetFilePointer(fd, 0, NULL, 0);
            if (fd == 0)
            {
                *handleOut = 0;
                return 0;
            }
            handle = new CFileHandle;
            if (handle != NULL)
            {
                handle->mFileHandle = fd;
                handle->mLength = 0;
                handle->mStartOffset = 0;
                handle->mFileSize = size;
                handle->mValidPosition = 1;
                strncpy(handle->mFilename, localName, 0x104);
            }
        }

        dst = localName;
        while (*p != '*')
        {
            *dst++ = *p++;
        }
        *dst = '\0';
        handle->mLength = atoi(localName);
        p++;

        dst = localName;
        while (*p != '.')
        {
            *dst++ = *p++;
        }
        *dst = '\0';
        int length = atoi(localName);
        handle->mCurrentPos = length;
        handle->mStartOffset = length;
        handle->mValidPosition = 0;
        InvalidePositionsForOtherHandles(handle);
        EnterCriticalSection(&sMutex);
        sFileHandles.push_back(handle);
        LeaveCriticalSection(&sMutex);
        *handleOut = (unsigned int)handle;
        return 1;
    }

    int fd = CreateFileA(filename, 0x80000000, 0, NULL, 3, 0, 0);
    if (fd != 0)
    {
        int size = SetFilePointer(fd, 0, NULL, 2);
        SetFilePointer(fd, 0, NULL, 0);
        CFileHandle* handle = new CFileHandle;
        if (handle != NULL)
        {
            handle->mCurrentPos = 0;
            handle->mFileHandle = fd;
            handle->mLength = 0;
            handle->mStartOffset = 0;
            handle->mFileSize = size;
            handle->mValidPosition = 1;
            strncpy(handle->mFilename, filename, 0x104);
        }
        EnterCriticalSection(&sMutex);
        sFileHandles.push_back(handle);
        LeaveCriticalSection(&sMutex);
        *handleOut = (unsigned int)handle;
        return 1;
    }

    *handleOut = 0;
    return 0;
}

void file_close_callback(unsigned int handleVal)
{
    CFileHandle* handle = (CFileHandle*)handleVal;
    int fd = handle->mFileHandle;

    EnterCriticalSection(&sMutex);
    for (size_t i = 0; i < sFileHandles.size(); ++i)
    {
        if (sFileHandles[i] == handle)
        {
            delete handle;
            sFileHandles[i] = sFileHandles.back();
            sFileHandles.pop_back();
            break;
        }
    }

    for (size_t i = 0; i < sFileHandles.size(); ++i)
    {
        if (sFileHandles[i]->mFileHandle == fd)
        {
            LeaveCriticalSection(&sMutex);
            return;
        }
    }
    LeaveCriticalSection(&sMutex);
    CloseHandle(fd);
}

int file_seek_callback(unsigned int handleVal, int offset, unsigned int origin)
{
    CFileHandle* handle = (CFileHandle*)handleVal;

    int length = handle->mLength;
    if (length != 0)
    {
        unsigned int target;
        unsigned int maxPos = handle->mStartOffset + length;
        if (origin == 1)
        {
            target = handle->mCurrentPos + offset;
            if (target > maxPos)
                target = maxPos;
        }
        else if (origin == 2)
        {
            target = maxPos + offset;
            if (target > maxPos)
                target = maxPos;
        }
        else
        {
            target = handle->mStartOffset + offset;
            if (target > maxPos)
                target = maxPos;
        }
        handle->mValidPosition = 0;
        handle->mCurrentPos = target;
        InvalidePositionsForOtherHandles(handle);
        return handle->mCurrentPos - handle->mStartOffset;
    }

    unsigned int target;
    unsigned int maxPos = handle->mFileSize;
    if (origin == 1)
    {
        target = handle->mCurrentPos + offset;
        if (target > maxPos)
            target = maxPos;
    }
    else if (origin == 2)
    {
        target = maxPos + offset;
        if (target > maxPos)
            target = maxPos;
    }
    else
    {
        target = offset;
        if (target > maxPos)
            target = maxPos;
    }
    handle->mValidPosition = 0;
    handle->mCurrentPos = target;
    InvalidePositionsForOtherHandles(handle);
    return handle->mCurrentPos;
}

int file_read_callback(unsigned int handleVal, void* buffer, unsigned int bytes)
{
    CFileHandle* handle = (CFileHandle*)handleVal;

    if (handle->mValidPosition == 0)
    {
        SetFilePointer(handle->mFileHandle, handle->mCurrentPos, NULL, 0);
        handle->mValidPosition = 1;
    }

    if (handle->mLength != 0)
    {
        int maxPos = handle->mStartOffset + handle->mLength;
        if ((unsigned int)maxPos < (unsigned int)(handle->mCurrentPos + bytes))
        {
            bytes = maxPos - handle->mCurrentPos;
        }
    }

    unsigned int bytesRead;
    int success = ReadFile(handle->mFileHandle, buffer, bytes, &bytesRead, NULL);
    if (success != 0)
    {
        handle->mCurrentPos += bytesRead;
        InvalidePositionsForOtherHandles(handle);
        return bytesRead;
    }
    return 0;
}
