#include "game/core/FileHeader.hpp"

#include <string.h>
#include <wchar.h>

#include <algorithm>

wchar_t* wstringtochar16(const std::wstring& string);
std::wstring char16towstring(wchar_t* string);

wchar_t* wstringtochar16(const std::wstring& string)
{
    return const_cast<wchar_t*>(string.c_str());
}

static void reverseUnsignedInt(unsigned int* value)
{
    unsigned int v = *value;
    *value = ((v & 0x000000ffU) << 24) |
        ((v & 0x0000ff00U) << 8) |
        ((v & 0x00ff0000U) >> 8) |
        ((v & 0xff000000U) >> 24);
}

static void reverseShort(short* value)
{
    unsigned short v = (unsigned short)*value;
    *value = (short)((v << 8) | (v >> 8));
}

static void reverseLongLong(long long* value)
{
    unsigned long long v = (unsigned long long)*value;
    v = ((v & 0x00000000000000ffULL) << 56) |
        ((v & 0x000000000000ff00ULL) << 40) |
        ((v & 0x0000000000ff0000ULL) << 24) |
        ((v & 0x00000000ff000000ULL) << 8) |
        ((v & 0x000000ff00000000ULL) >> 8) |
        ((v & 0x0000ff0000000000ULL) >> 24) |
        ((v & 0x00ff000000000000ULL) >> 40) |
        ((v & 0xff00000000000000ULL) >> 56);
    *value = (long long)v;
}

static void reverseChar16Array(wchar_t* value, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        unsigned short v = (unsigned short)value[i];
        value[i] = (wchar_t)((v << 8) | (v >> 8));
    }
}

static const unsigned int SAVE_HEADER_SIZE = 12;
static const unsigned int FILE_ENTRY_DATA_SIZE = 144;
static const unsigned int FILE_ENTRY_DATA_SIZE_V1 = 136;
static const short CURRENT_SAVE_VERSION = 11;
static const short MAX_SUPPORTED_SAVE_VERSION = 12;

ByteOrder FileHeader::getNativeEndian()
{
    const unsigned int value = 1;
    return *reinterpret_cast<const unsigned char*>(&value) == 1
        ? eByteOrder_LittleEndian : eByteOrder_BigEndian;
}

FileHeader::FileHeader()
{
    m_savePlatform = eSavePlatform_WIIU;
    m_saveEndian = eByteOrder_BigEndian;
    m_saveVersion = 0;
    m_originalSaveVersion = CURRENT_SAVE_VERSION;
    m_currentFile = NULL;
}

FileHeader::~FileHeader()
{
    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        if (m_fileEntries[i] != NULL)
            delete m_fileEntries[i];
    }
}

void FileHeader::Reset()
{
    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        if (m_fileEntries[i] != NULL)
            delete m_fileEntries[i];
    }

    m_fileEntries.clear();

    m_savePlatform = eSavePlatform_WIIU;
    m_saveEndian = eByteOrder_BigEndian;
    m_saveVersion = 0;
    m_originalSaveVersion = CURRENT_SAVE_VERSION;
    m_currentFile = NULL;
}

unsigned int FileHeader::GetStartOfNextData()
{
    unsigned int startOfNextData = SAVE_HEADER_SIZE;

    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        if (m_fileEntries[i]->m_fileSize != 0)
            startOfNextData += m_fileEntries[i]->m_fileSize;
    }

    return startOfNextData;
}

FileEntry* FileHeader::AddFile(const std::wstring& fileName, unsigned int fileSize)
{
    wchar_t name[64];

    memset(name, 0, sizeof(name));

    unsigned int nameSize = fileName.length() * sizeof(wchar_t);
    if (nameSize > sizeof(name))
        nameSize = sizeof(name);

    memcpy(name, wstringtochar16(fileName), nameSize);

    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        if (m_fileEntries[i]->m_fileName.compare(fileName) == 0)
            return m_fileEntries[i];
    }

    m_fileEntries.push_back(new FileEntry(name, fileSize, GetStartOfNextData()));
    m_currentFile = m_fileEntries[m_fileEntries.size() - 1];

    return m_currentFile;
}

void FileHeader::AdjustStartOffsets(FileEntry* entry, unsigned int size, bool bRemove)
{
    bool bFound = false;

    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        if (bFound)
        {
            if (bRemove)
            {
                m_fileEntries[i]->m_startOffset -= size;
                m_fileEntries[i]->m_filePointer -= size;
            }
            else
            {
                m_fileEntries[i]->m_startOffset += size;
                m_fileEntries[i]->m_filePointer += size;
            }
        }
        else if (m_fileEntries[i] == entry)
        {
            bFound = true;
        }
    }
}

void FileHeader::RemoveFile(FileEntry* entry)
{
    if (entry == NULL)
        return;

    AdjustStartOffsets(entry, entry->m_fileSize, true);

    std::vector<FileEntry*>::iterator it = std::find(m_fileEntries.begin(), m_fileEntries.end(), entry);
    if (it < m_fileEntries.end())
        m_fileEntries.erase(it);

    delete entry;
}

void FileHeader::WriteHeader(void* pData)
{
    unsigned int indexOffset = GetStartOfNextData();

    unsigned int* pOffsets = (unsigned int*)pData;
    short* pVersions = (short*)(pOffsets + 2);

    pOffsets[0] = indexOffset;
    pOffsets[1] = m_fileEntries.size();
    pVersions[0] = m_originalSaveVersion;
    pVersions[1] = CURRENT_SAVE_VERSION;

    if (getNativeEndian() != eByteOrder_BigEndian)
    {
        reverseUnsignedInt(&pOffsets[0]);
        reverseUnsignedInt(&pOffsets[1]);
        reverseShort(&pVersions[0]);
        reverseShort(&pVersions[1]);
    }

    unsigned char* pEntryData = (unsigned char*)pData + indexOffset;

    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        memcpy(pEntryData, m_fileEntries[i], FILE_ENTRY_DATA_SIZE);
        if (getNativeEndian() != eByteOrder_BigEndian)
        {
            reverseChar16Array(reinterpret_cast<wchar_t*>(pEntryData), 64);
            reverseUnsignedInt(reinterpret_cast<unsigned int*>(pEntryData + 128));
            reverseUnsignedInt(reinterpret_cast<unsigned int*>(pEntryData + 132));
            reverseLongLong(reinterpret_cast<long long*>(pEntryData + 136));
        }
        pEntryData += FILE_ENTRY_DATA_SIZE;
    }
}

void FileHeader::ReadHeader(void* pData, ESavePlatform platform)
{
    switch (platform)
    {
    case eSavePlatform_X360:
    case eSavePlatform_WIIU:
    case eSavePlatform_PS3:
        m_savePlatform = platform;
        m_saveEndian = eByteOrder_BigEndian;
        break;

    case eSavePlatform_NX:
    case eSavePlatform_XB1:
    case eSavePlatform_PS4:
    case eSavePlatform_WIN:
    case eSavePlatform_PSV:
        m_savePlatform = platform;
        m_saveEndian = eByteOrder_LittleEndian;
        break;

    default:
        m_savePlatform = eSavePlatform_WIIU;
        m_saveEndian = eByteOrder_BigEndian;
        break;
    }

    unsigned int* pOffsets = (unsigned int*)pData;
    short* pVersions = (short*)(pOffsets + 2);

    unsigned int indexOffset = pOffsets[0];
    if (m_saveEndian != getNativeEndian())
        reverseUnsignedInt(&indexOffset);

    unsigned int fileCount = pOffsets[1];
    if (m_saveEndian != getNativeEndian())
        reverseUnsignedInt(&fileCount);

    m_originalSaveVersion = pVersions[0];
    if (m_saveEndian != getNativeEndian())
        reverseShort(&m_originalSaveVersion);

    m_saveVersion = pVersions[1];
    if (m_saveEndian != getNativeEndian())
        reverseShort(&m_saveVersion);

    unsigned char* pEntryData = (unsigned char*)pData + indexOffset;

    if (m_saveVersion == 0)
        return;

    if (m_saveVersion == 1)
    {
        unsigned int readOffset = 0;

        while (readOffset < fileCount)
        {
            FileEntry* pEntry = new FileEntry();

            memcpy(pEntry, pEntryData, FILE_ENTRY_DATA_SIZE_V1);

            m_currentFile = pEntry;
            pEntry->m_filePointer = pEntry->m_startOffset;
            m_fileEntries.push_back(pEntry);

            readOffset += FILE_ENTRY_DATA_SIZE_V1;
            pEntryData += FILE_ENTRY_DATA_SIZE_V1;
        }
    }
    else if (m_saveVersion <= MAX_SUPPORTED_SAVE_VERSION)
    {
        for (unsigned int i = 0; i < fileCount; i++)
        {
            FileEntry* pEntry = new FileEntry();

            memcpy(pEntry, pEntryData, FILE_ENTRY_DATA_SIZE);

            if (m_saveEndian != getNativeEndian())
            {
                reverseChar16Array(pEntry->m_name, 64);
                reverseUnsignedInt(&pEntry->m_fileSize);
                reverseUnsignedInt(&pEntry->m_startOffset);
                reverseLongLong(&pEntry->m_timestamp);
            }

            pEntry->m_fileName = char16towstring(pEntry->m_name);

            m_currentFile = pEntry;
            pEntry->m_filePointer = pEntry->m_startOffset;
            m_fileEntries.push_back(pEntry);

            pEntryData += FILE_ENTRY_DATA_SIZE;
        }
    }
}

unsigned int FileHeader::GetFileSize()
{
    return GetStartOfNextData() + m_fileEntries.size() * FILE_ENTRY_DATA_SIZE;
}

bool FileHeader::fileExists(const std::wstring& fileName)
{
    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        if (m_fileEntries[i]->m_fileName.compare(fileName) == 0)
            return true;
    }

    return false;
}

std::vector<FileEntry*>* FileHeader::getFilesWithPrefix(const std::wstring& prefix)
{
    std::vector<FileEntry*>* pFiles = NULL;

    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        if (wcsncmp(m_fileEntries[i]->m_fileName.c_str(), prefix.c_str(), prefix.length()) == 0)
        {
            if (pFiles == NULL)
                pFiles = new std::vector<FileEntry*>();

            pFiles->push_back(m_fileEntries[i]);
        }
    }

    return pFiles;
}

ByteOrder FileHeader::getEndian(ESavePlatform platform)
{
    switch (platform)
    {
    case eSavePlatform_X360:
    case eSavePlatform_WIIU:
    case eSavePlatform_PS3:
        return eByteOrder_BigEndian;

    default:
        return eByteOrder_LittleEndian;
    }
}
