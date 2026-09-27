#include "game/core/ArchiveFile.hpp"
#include <cstring>
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/Compression.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/FileInputStream.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"

struct _OVERLAPPED
{
    unsigned int Internal;
    unsigned int InternalHigh;
    unsigned int Offset;
    unsigned int OffsetHigh;
    int hEvent;
};

char* wstringtofilename(const std::wstring& path, char (&filename)[256]);
int CreateFileA(const char* fileName, unsigned int desiredAccess, unsigned int shareMode,
                void* securityAttributes, unsigned int creationDisposition,
                unsigned int flagsAndAttributes, int templateFile);
unsigned int ReadFile(int file, void* buffer, unsigned int bytesToRead,
                      unsigned int* bytesRead, _OVERLAPPED* overlapped);
int SetFilePointer(int file, long distanceToMove, int* distanceToMoveHigh,
                   unsigned int moveMethod);

static const int INVALID_HANDLE_VALUE = -1;
static const unsigned int GENERIC_READ = 0x80000000;
static const unsigned int OPEN_EXISTING = 3;
static const unsigned int FILE_ATTRIBUTE_NORMAL = 0x00000080;

ArchiveFile::ArchiveFile(File file)
{
    m_data = NULL;
    m_cachedData = false;
    File::operator=(file);
    if (!file.exists()) gApp.FatalLoadError(L"<ArchiveFile@%d> Failed to load ", 0x54);
    FileInputStream input(file);
    DataInputStream stream(&input);
    _readHeader(&stream);
    stream.close();
    input.close();
}

ArchiveFile::ArchiveFile(unsigned char* data, unsigned int size)
{
    m_data = data;
    m_cachedData = true;
    arrayWithLength<unsigned char> bytes;
    bytes.data = data;
    bytes.length = size;
    ByteArrayInputStream input(bytes);
    DataInputStream stream((InputStream*)&input);
    _readHeader(&stream);
    stream.close();
    input.mBuffer.data = NULL;
    input.mBuffer.length = 0;
    input.mCount = 0;
    input.mMark = 0;
    input.mPosition = 0;
}

ArchiveFile::~ArchiveFile()
{
    boost::unordered_map<std::wstring, _MetaData*>::iterator iterator = m_files.begin();
    while (iterator != m_files.end())
    {
        delete iterator->second;
        ++iterator;
    }
    m_files.clear();
    dereferenceCachedData();
}

void ArchiveFile::_readHeader(DataInputStream* stream)
{
    int count = stream->readInt();
    while (count > 0)
    {
        _MetaData* metadata = new _MetaData;
        metadata->name = stream->readUTF();
        metadata->offset = static_cast<unsigned int>(stream->readInt());
        metadata->size = static_cast<unsigned int>(stream->readInt());
        metadata->compressed = false;
        if (!metadata->name.empty() && metadata->name[0] == L'*')
        {
            metadata->name.erase(0, 1);
            metadata->compressed = true;
        }
        m_files.insert(std::make_pair(metadata->name, metadata));
        --count;
    }
}

bool ArchiveFile::hasFile(const std::wstring& name)
{
    return m_files.find(name) != m_files.end();
}

arrayWithLength<unsigned char> ArchiveFile::getFile(const std::wstring& name)
{
    arrayWithLength<unsigned char> result;
    boost::unordered_map<std::wstring, _MetaData*>::iterator found = m_files.find(name);
    if (found == m_files.end())
    {
        gApp.FatalLoadError(L"<ArchiveFile@%d> Failed to find ", 0xB1, name.c_str(), getPath().c_str());
        return result;
    }
    _MetaData* metadata = found->second;
    if (m_cachedData)
    {
        result = arrayWithLength<unsigned char>(metadata->size, true);
        std::memcpy(result.data, m_data + metadata->offset, metadata->size);
    }
    else
    {
        char filename[256];
        int file = CreateFileA(wstringtofilename(getPath(), filename), GENERIC_READ, 0, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (file == INVALID_HANDLE_VALUE)
        {
            gApp.FatalLoadError(L"<ArchiveFile@%d> Failed to load ", 0xFD, name.c_str(),
                                getPath().c_str());
        }
        else if (SetFilePointer(file, metadata->offset, NULL, 0) == 0)
        {
            gApp.FatalLoadError(L"<ArchiveFile@%d> Failed to set f", 0xF5, name.c_str());
            CloseHandle(file);
        }
        else
        {
            result = arrayWithLength<unsigned char>(metadata->size, true);
            unsigned int bytesRead = 0;
            if (ReadFile(file, result.data, metadata->size, &bytesRead, NULL) == 0)
            {
                gApp.FatalLoadError(L"<ArchiveFile@%d> Failed to read ", 0xEE, name.c_str(),
                                    getPath().c_str());
            }
            CloseHandle(file);
        }
    }
    if (metadata->compressed && result.data != NULL)
    {
        ByteArrayInputStream input(result);
        DataInputStream stream((InputStream*)&input);
        unsigned int decompressedSize = static_cast<unsigned int>(stream.readInt());
        stream.close();
        arrayWithLength<unsigned char> decompressed(decompressedSize, true);
        Compression::getCompression()->Decompress(decompressed.data, &decompressedSize, result.data + 4, result.length - 4);
        input.mBuffer.data = NULL;
        input.mBuffer.length = 0;
        input.mCount = 0;
        input.mMark = 0;
        input.mPosition = 0;
        delete[] result.data;
        result = decompressed;
        result.length = decompressedSize;
    }
    return result;
}

unsigned int ArchiveFile::getFileSize(const std::wstring& name)
{
    if (!hasFile(name)) return static_cast<unsigned int>(-1);
    return m_files.at(name)->size;
}

std::vector<std::wstring> ArchiveFile::getFileList()
{
    std::vector<std::wstring> result;
    boost::unordered_map<std::wstring, _MetaData*>::const_iterator iterator = m_files.begin();
    while (iterator != m_files.end())
    {
        result.push_back(iterator->first);
        ++iterator;
    }
    return result;
}

void ArchiveFile::dereferenceCachedData()
{
    if (m_cachedData && m_data != NULL) delete[] m_data;
    m_data = NULL;
    m_cachedData = false;
}
