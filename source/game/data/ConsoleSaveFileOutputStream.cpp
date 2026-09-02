#include "game/data/ConsoleSaveFileOutputStream.hpp"

ConsoleSaveFileOutputStream::ConsoleSaveFileOutputStream(ConsoleSaveFile* saveFile, const ConsoleSavePath& path)
{
    mSaveFile = saveFile;
    mFileEntry = saveFile->createFile(path);
    mSaveFile->setFilePointer(mFileEntry, 0, NULL, 0);
}

ConsoleSaveFileOutputStream::ConsoleSaveFileOutputStream(ConsoleSaveFile* saveFile, FileEntry* fileEntry)
{
    mFileEntry = fileEntry;
    mSaveFile = saveFile;
    saveFile->setFilePointer(fileEntry, 0, NULL, 0);
}

ConsoleSaveFileOutputStream::~ConsoleSaveFileOutputStream()
{
}

void ConsoleSaveFileOutputStream::write(unsigned int value)
{
    unsigned char byteValue = (unsigned char)value;
    unsigned int bytesWritten;

    mSaveFile->writeFile(mFileEntry, &byteValue, 1, &bytesWritten);
}

void ConsoleSaveFileOutputStream::write(arrayWithLength<unsigned char> data)
{
    unsigned int bytesWritten;

    mSaveFile->writeFile(mFileEntry, &data, data.length, &bytesWritten);
}

void ConsoleSaveFileOutputStream::write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length)
{
    unsigned int bytesWritten;

    mSaveFile->writeFile(mFileEntry, data.data + offset, length, &bytesWritten);
}

void ConsoleSaveFileOutputStream::close()
{
    if (mSaveFile != NULL)
    {
        mSaveFile->closeHandle(mFileEntry);
        mSaveFile = NULL;
    }
}

void ConsoleSaveFileOutputStream::flush()
{
}
