#include "vendor/MilesIO.hpp"

#include <stddef.h>
#include <string.h>

#include "vendor/MilesAsync.hpp"
#include "vendor/MilesMem.hpp"
#include "vendor/MilesRR.hpp"

static const unsigned int MILES_IO_WHOLE_FILE = 0xFFFFFFFF;
static const unsigned int MILES_IO_SEEK_SET = 0;
static const unsigned int MILES_IO_SEEK_CURRENT = 1;
static const unsigned int MILES_IO_SEEK_END = 2;

static const char MILES_IO_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\io.cpp";

AIL_FILE_OPEN_CALLBACK AIL_file_open = DefaultFileOpen;
AIL_FILE_CLOSE_CALLBACK AIL_file_close = DefaultFileClose;
AIL_FILE_SEEK_CALLBACK AIL_file_seek = DefaultFileSeek;
AIL_FILE_READ_CALLBACK AIL_file_read = DefaultFileRead;

static const char* ParseSubFileNumber(const char* pCursor, unsigned int* puValue)
{
    unsigned int uValue;
    int bNegative;
    int iCharacter;

    iCharacter = (int)*++pCursor;

    bNegative = iCharacter == '-';
    if (bNegative)
    {
        iCharacter = (int)*++pCursor;
    }

    uValue = 0;
    while ((unsigned int)(iCharacter - '0') < 10)
    {
        uValue = uValue * 10 + (unsigned int)(iCharacter - '0');
        iCharacter = (int)*++pCursor;
    }

    if (bNegative)
    {
        uValue = (unsigned int)(-(int)uValue);
    }

    *puValue = uValue;
    return pCursor;
}

unsigned int DefaultFileOpen(const char* pFileName, unsigned int* puFileHandle)
{
    MilesFile* pFile;
    const char* pCursor;
    char* pOutput;
    char szName[512];
    unsigned int uLength;
    unsigned int uOffset;
    unsigned int uSize;
    int iCharacter;
    int iOpened;

    pFile = (MilesFile*)AIL_mem_alloc_lock_info((int)sizeof(MilesFile), MILES_IO_FILE, 71);
    if (pFile == NULL)
    {
        return 0;
    }

    memset(pFile, 0, sizeof(MilesFile));

    uLength = MILES_IO_WHOLE_FILE;
    uOffset = 0;

    if (*pFileName == '*')
    {
        pCursor = pFileName;
        pOutput = szName;

        for (;;)
        {
            iCharacter = (int)*++pCursor;
            if (iCharacter == '\0' || iCharacter == '*')
            {
                break;
            }

            *pOutput = (char)iCharacter;
            pOutput = pOutput + 1;
        }

        *pOutput = '\0';

        if (*pCursor == '*')
        {
            pCursor = ParseSubFileNumber(pCursor, &uLength);

            if (*pCursor == '*')
            {
                pCursor = ParseSubFileNumber(pCursor, &uOffset);
            }
        }

        iOpened = Platform_OpenFile(pFile, szName);
    }
    else
    {
        iOpened = Platform_OpenFile(pFile, pFileName);
    }

    if (iOpened == 0)
    {
        AIL_mem_free_lock(pFile);
        return 0;
    }

    uSize = pFile->uSize;
    if (uOffset < uSize)
    {
        if (uLength > uSize - uOffset)
        {
            uLength = uSize - uOffset;
        }
    }
    else
    {
        uOffset = uSize;
        uLength = 0;
    }

    pFile->uLength = uLength;

    if (uOffset != MILES_IO_WHOLE_FILE)
    {
        pFile->uBaseOffset = uOffset;
    }

    *puFileHandle = (unsigned int)pFile;
    return 1;
}

void DefaultFileClose(unsigned int uFileHandle)
{
    MilesFile* pFile;

    pFile = (MilesFile*)uFileHandle;
    if (pFile == NULL)
    {
        return;
    }

    if (pFile->iDoNotClose == 0)
    {
        Platform_CloseFile(pFile);
    }

    AIL_mem_free_lock(pFile);
}

int DefaultFileSeek(unsigned int uFileHandle, int iOffset, unsigned int uType)
{
    MilesFile* pFile;
    unsigned int uLength;
    unsigned int uPosition;

    pFile = (MilesFile*)uFileHandle;
    if (pFile == NULL)
    {
        return 0;
    }

    if (uType == MILES_IO_SEEK_END)
    {
        uLength = pFile->uLength;
        uPosition = uLength + (unsigned int)iOffset;
    }
    else
    {
        uPosition = (unsigned int)iOffset;
        if (uType == MILES_IO_SEEK_CURRENT)
        {
            uPosition = pFile->uPosition + uPosition;
        }

        uLength = pFile->uLength;
    }

    pFile->uPosition = uPosition;

    if (uPosition > uLength)
    {
        pFile->uPosition = uLength;
        uPosition = uLength;
    }

    Platform_SeekFromBeginning(pFile, uPosition);
    return (int)pFile->uPosition;
}

int DefaultFileRead(unsigned int uFileHandle, void* pBuffer, unsigned int uBytes)
{
    MilesFile* pFile;
    int iBytesRead;

    pFile = (MilesFile*)uFileHandle;
    if (pFile == NULL)
    {
        return 0;
    }

    if (pFile->uPosition + uBytes > pFile->uLength)
    {
        uBytes = pFile->uLength - pFile->uPosition;
    }

    iBytesRead = Platform_ReadFile(pFile, pBuffer, uBytes);
    pFile->uPosition = pFile->uPosition + (unsigned int)iBytesRead;
    return iBytesRead;
}

void CloseAsync(MilesAsyncRead* pRead)
{
    if (pRead->uFileHandle == 0)
    {
        return;
    }

    pRead->iLength = 0;
    pRead->iLengthHigh = 0;
    pRead->uFlags = 0;

    MilesAsyncFileRead(pRead);
    MilesAsyncFileStatus(pRead, MILES_IO_WHOLE_FILE);
}
