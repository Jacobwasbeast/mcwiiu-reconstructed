#include "game/core/StbiFile.hpp"

#include <stddef.h>

struct _OVERLAPPED
{
    unsigned int Internal;
    unsigned int InternalHigh;
    unsigned int Offset;
    unsigned int OffsetHigh;
    int hEvent;
};

int ReadFile(int file, void* buffer, unsigned int bytesToRead, unsigned int* bytesRead, _OVERLAPPED* overlapped);
int SetFilePointer(int file, long distanceToMove, int* distanceToMoveHigh, unsigned int moveMethod);

static const unsigned int FILE_CURRENT = 1;

int StbiFile::read(void* user, char* data, int size)
{
    StbiFile* stbiFile = (StbiFile*)user;

    unsigned int bytesRead = 0;

    if (ReadFile(stbiFile->mFile, data, size, &bytesRead, NULL) < 0)
    {
        return -1;
    }

    stbiFile->mFilePosition += bytesRead;

    return bytesRead;
}

void StbiFile::skip(void* user, int count)
{
    StbiFile* stbiFile = (StbiFile*)user;

    SetFilePointer(stbiFile->mFile, count, NULL, FILE_CURRENT);

    stbiFile->mFilePosition += count;
}

int StbiFile::eof(void* user)
{
    StbiFile* stbiFile = (StbiFile*)user;

    return (stbiFile->mFileSize - stbiFile->mFilePosition) <= 0;
}
