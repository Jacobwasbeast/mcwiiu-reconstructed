#include "platform/wiiu/WiiUMilesFileSystem.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"
#include "game/core/CriticalSection.hpp"
#include <vector>
#include <cstdlib>
#include <cstring>

extern int ReadFile(int hFile, void* lpBuffer, unsigned int nNumberOfBytesToRead, unsigned int* lpNumberOfBytesRead, void* lpOverlapped);

struct CFileHandle
{
    char mFilename[0x104];
    int mFileHandle;
    int mStartOffset;
    int mLength;
    int mCurrentPos;
    char mValidPosition;
    char mPadding[3];
    int mFileSize;
};

static char sInitialized = 0;
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

int file_open_callback(const char* filename, unsigned int* handleOut)
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
        while (*p != '*' && *p != '\0')
        {
            *dst++ = *p++;
        }
        *dst = '\0';
        if (*p == '*')
            p++;

        CFileHandle* existingHandle = NULL;
        EnterCriticalSection(&sMutex);
        for (size_t i = 0; i < sFileHandles.size(); ++i)
        {
            if (strcmp(sFileHandles[i]->mFilename, filename) == 0)
            {
                existingHandle = sFileHandles[i];
                break;
            }
        }
        LeaveCriticalSection(&sMutex);

        if (existingHandle != NULL)
        {
            CFileHandle* newHandle = new CFileHandle();
            if (newHandle != NULL)
            {
                newHandle->mFileSize = existingHandle->mFileSize;
                newHandle->mStartOffset = 0;
                newHandle->mCurrentPos = 0;
                newHandle->mValidPosition = 1;
                newHandle->mFileHandle = existingHandle->mFileHandle;
                newHandle->mLength = 0;
                strncpy(newHandle->mFilename, filename, 0x104);
            }
            char numBuf[260];
            char* numDst = numBuf;
            while (*p != '*' && *p != '\0')
            {
                *numDst++ = *p++;
            }
            *numDst = '\0';
            if (*p == '*')
                p++;
            int start = atoi(numBuf);
            if (newHandle)
                newHandle->mLength = start;

            numDst = numBuf;
            while (*p != '.' && *p != '\0')
            {
                *numDst++ = *p++;
            }
            *numDst = '\0';
            int length = atoi(numBuf);
            if (newHandle)
            {
                newHandle->mCurrentPos = length;
                newHandle->mStartOffset = length;
                newHandle->mValidPosition = 0;
                InvalidePositionsForOtherHandles(newHandle);
                EnterCriticalSection(&sMutex);
                sFileHandles.push_back(newHandle);
                LeaveCriticalSection(&sMutex);
                *handleOut = (unsigned int)newHandle;
                return 1;
            }
            *handleOut = 0;
            return 0;
        }

        int fd = CreateFileA(localName, 0x80000000, 0, NULL, 3, 0, 0);
        if (fd != 0)
        {
            int size = SetFilePointer(fd, 0, NULL, 2);
            SetFilePointer(fd, 0, NULL, 0);
            CFileHandle* newHandle = new CFileHandle();
            if (newHandle != NULL)
            {
                newHandle->mFileHandle = fd;
                newHandle->mLength = 0;
                newHandle->mStartOffset = 0;
                newHandle->mFileSize = size;
                newHandle->mValidPosition = 1;
                strncpy(newHandle->mFilename, localName, 0x104);
            }
            char numBuf[260];
            char* numDst = numBuf;
            while (*p != '*' && *p != '\0')
            {
                *numDst++ = *p++;
            }
            *numDst = '\0';
            if (*p == '*')
                p++;
            int start = atoi(numBuf);
            if (newHandle)
                newHandle->mLength = start;

            numDst = numBuf;
            while (*p != '.' && *p != '\0')
            {
                *numDst++ = *p++;
            }
            *numDst = '\0';
            int length = atoi(numBuf);
            if (newHandle)
            {
                newHandle->mCurrentPos = length;
                newHandle->mStartOffset = length;
                newHandle->mValidPosition = 0;
                InvalidePositionsForOtherHandles(newHandle);
                EnterCriticalSection(&sMutex);
                sFileHandles.push_back(newHandle);
                LeaveCriticalSection(&sMutex);
                *handleOut = (unsigned int)newHandle;
                return 1;
            }
        }
        *handleOut = 0;
        return 0;
    }

    int fd = CreateFileA(filename, 0x80000000, 0, NULL, 3, 0, 0);
    if (fd != 0)
    {
        int size = SetFilePointer(fd, 0, NULL, 2);
        SetFilePointer(fd, 0, NULL, 0);
        CFileHandle* handle = new CFileHandle();
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
    if (!handle)
        return;
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

int file_seek_callback(unsigned int handleVal, int offset, int origin)
{
    CFileHandle* handle = (CFileHandle*)handleVal;
    if (!handle)
        return 0;

    int length = handle->mLength;
    if (length != 0)
    {
        int target;
        int maxPos = handle->mStartOffset + length;
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

    int target;
    int maxPos = handle->mFileSize;
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

int file_read_callback(unsigned int handleVal, void* buffer, int bytes)
{
    CFileHandle* handle = (CFileHandle*)handleVal;
    if (!handle)
        return 0;

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

    unsigned int bytesRead = 0;
    int success = ReadFile(handle->mFileHandle, buffer, bytes, &bytesRead, NULL);
    if (success != 0)
    {
        handle->mCurrentPos += bytesRead;
        InvalidePositionsForOtherHandles(handle);
        return bytesRead;
    }
    return 0;
}
