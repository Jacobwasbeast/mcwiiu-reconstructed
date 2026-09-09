#include "game/core/ArchiveFile.hpp"
#include <cstring>
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/Compression.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/FileInputStream.hpp"

struct _OVERLAPPED;

extern int wstringtofilename(const std::wstring&, char (&)[256]);
extern int CreateFileA(const char*, unsigned int, unsigned int, void*, unsigned int, unsigned int, int);
extern int SetFilePointer(int, long, int*, unsigned int);
extern int ReadFile(int, void*, unsigned int, unsigned int*, _OVERLAPPED*);
extern int CloseHandle(int);

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
        wstringtofilename(getPath(), filename);
        int handle = CreateFileA(filename, 0x80000000, 0, NULL, 3, 0x80, 0);
        if (handle == -1 || SetFilePointer(handle, static_cast<int>(metadata->offset), NULL, 0) == 0)
        {
            gApp.FatalLoadError(L"<ArchiveFile@%d> Failed to set f", 0xF5, name.c_str());
            if (handle != -1) CloseHandle(handle);
            return result;
        }
        result = arrayWithLength<unsigned char>(metadata->size, true);
        unsigned int read = 0;
        if (!ReadFile(handle, result.data, metadata->size, &read, NULL))
        {
            gApp.FatalLoadError(L"<ArchiveFile@%d> Failed to read ", 0xEE, name.c_str(), getPath().c_str());
            delete[] result.data;
            result.data = NULL;
            result.length = 0;
        }
        CloseHandle(handle);
    }
    if (metadata->compressed && result.data != NULL)
    {
        ByteArrayInputStream input(result);
        DataInputStream stream((InputStream*)&input);
        unsigned int decompressedSize = static_cast<unsigned int>(stream.readInt());
        stream.close();
        arrayWithLength<unsigned char> decompressed(decompressedSize, true);
        Compression::getCompression()->Decompress(decompressed.data, &decompressedSize, result.data + 4, result.length - 4);
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
