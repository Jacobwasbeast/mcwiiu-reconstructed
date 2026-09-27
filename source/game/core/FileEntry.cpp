#include "game/core/FileEntry.hpp"
#include "game/core/StringConversion.hpp"

#include <string.h>

FileEntry::FileEntry(wchar_t* name, unsigned int fileSize, unsigned int startOffset)
{
    memset(m_name, 0, sizeof(m_name));
    memcpy(m_name, name, sizeof(m_name));
    m_fileName = char16towstring(name);
    m_fileSize = fileSize;
    m_startOffset = startOffset;
    m_timestamp = 0;
    m_filePointer = m_startOffset;
    m_isNewFile = true;
}

bool FileEntry::newestFirst(FileEntry* first, FileEntry* second)
{
    return first->m_timestamp > second->m_timestamp;
}
