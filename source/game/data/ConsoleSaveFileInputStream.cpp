#include "game/data/ConsoleSaveFileInputStream.hpp"

ConsoleSaveFileInputStream::ConsoleSaveFileInputStream(ConsoleSaveFile* saveFile,
                                                       const ConsoleSavePath& path)
    : mSaveFile(saveFile), mFileEntry(NULL)
{
    mFileEntry = mSaveFile->createFile(path);
    mSaveFile->setFilePointer(mFileEntry, 0, NULL, 0);
}

ConsoleSaveFileInputStream::ConsoleSaveFileInputStream(ConsoleSaveFile* saveFile,
                                                       FileEntry* fileEntry)
    : mSaveFile(saveFile), mFileEntry(fileEntry)
{
    mSaveFile->setFilePointer(mFileEntry, 0, NULL, 0);
}

int ConsoleSaveFileInputStream::read()
{
    unsigned char value = 0;
    unsigned int bytesRead;

    if (mSaveFile->readFile(mFileEntry, &value, 1, &bytesRead) && bytesRead != 0)
    {
        return value;
    }

    return -1;
}

int ConsoleSaveFileInputStream::read(arrayWithLength<unsigned char> data)
{
    unsigned int bytesRead;

    if (mSaveFile->readFile(mFileEntry, data.data, data.length, &bytesRead) && bytesRead != 0)
    {
        return bytesRead;
    }

    return -1;
}

int ConsoleSaveFileInputStream::read(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length)
{
    unsigned int bytesRead;

    if (mSaveFile->readFile(mFileEntry, data.data + offset, length, &bytesRead) && bytesRead != 0)
    {
        return bytesRead;
    }

    return -1;
}

void ConsoleSaveFileInputStream::close()
{
    if (mSaveFile != NULL)
    {
        mSaveFile->closeHandle(mFileEntry);
        mSaveFile = NULL;
    }
}

long long ConsoleSaveFileInputStream::skip(long long count)
{
    return count;
}
