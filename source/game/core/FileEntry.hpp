#ifndef GAME_CORE_FILEENTRY_HPP
#define GAME_CORE_FILEENTRY_HPP

#include <string.h>

#include <string>

class FileEntry
{
public:
    FileEntry()
    {
        memset(m_name, 0, sizeof(m_name));
        m_fileSize = 0;
        m_startOffset = 0;
        m_timestamp = 0;
    }

    FileEntry(wchar_t* name, unsigned int fileSize, unsigned int startOffset);

    static bool newestFirst(FileEntry* first, FileEntry* second);

    wchar_t m_name[64];
    unsigned int m_fileSize;
    unsigned int m_startOffset;
    long long m_timestamp;
    std::wstring m_fileName;
    unsigned int m_filePointer;
    bool m_isNewFile;
};

#endif
