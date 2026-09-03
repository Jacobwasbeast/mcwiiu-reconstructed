#include "game/data/ConsoleSaveFileOutputStream.hpp"

#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/ConsoleSavePath.hpp"
#include "game/core/FileEntry.hpp"

ConsoleSaveFileOutputStream::ConsoleSaveFileOutputStream(ConsoleSaveFile* saveFile, const ConsoleSavePath& path)
    : m_saveFile(saveFile)
{
    m_fileEntry = m_saveFile->createFile(path);
    m_saveFile->writeFile(m_fileEntry, NULL, 0, NULL);
}

ConsoleSaveFileOutputStream::ConsoleSaveFileOutputStream(ConsoleSaveFile* saveFile, FileEntry* fileEntry)
    : m_saveFile(saveFile), m_fileEntry(fileEntry)
{
    m_saveFile->writeFile(m_fileEntry, NULL, 0, NULL);
}

ConsoleSaveFileOutputStream::~ConsoleSaveFileOutputStream()
{
}

void ConsoleSaveFileOutputStream::write(unsigned int value)
{
    unsigned char byte = (unsigned char)value;
    unsigned int bytesWritten = 0;
    m_saveFile->writeFile(m_fileEntry, &byte, 1, &bytesWritten);
}

void ConsoleSaveFileOutputStream::write(arrayWithLength<unsigned char> data)
{
    unsigned int bytesWritten = 0;
    m_saveFile->writeFile(m_fileEntry, data.data, data.length, &bytesWritten);
}

void ConsoleSaveFileOutputStream::write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length)
{
    unsigned int bytesWritten = 0;
    m_saveFile->writeFile(m_fileEntry, data.data + offset, length, &bytesWritten);
}

void ConsoleSaveFileOutputStream::close()
{
    if (m_saveFile != NULL)
    {
        m_saveFile->closeHandle(m_fileEntry);
        m_saveFile = NULL;
    }
}

void ConsoleSaveFileOutputStream::flush()
{
}
