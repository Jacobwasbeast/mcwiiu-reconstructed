#include "game/core/FileOutputStream.hpp"

#include "game/core/File.hpp"

struct _OVERLAPPED
{
    unsigned int Internal;
    unsigned int InternalHigh;
    unsigned int Offset;
    unsigned int OffsetHigh;
    int hEvent;
};

char* wstringtofilename(const std::wstring& path, char (&filename)[256]);

int CreateFileA(const char* fileName, unsigned int desiredAccess, unsigned int shareMode, void* securityAttributes, unsigned int creationDisposition, unsigned int flagsAndAttributes, int templateFile);
int WriteFile(int file, const void* buffer, unsigned int bytesToWrite, unsigned int* bytesWritten, _OVERLAPPED* overlapped);
void CloseHandle(int handle);

static const int INVALID_HANDLE_VALUE = -1;
static const unsigned int GENERIC_WRITE = 0x40000000;
static const unsigned int OPEN_ALWAYS = 4;
static const unsigned int FILE_ATTRIBUTE_NORMAL = 0x80;

FileOutputStream::FileOutputStream(const File& file)
{
    mFile = INVALID_HANDLE_VALUE;

    if (!file.exists() || !file.isDirectory())
    {
        char buffer[256];
        char* fileName = wstringtofilename(file.getPath(), buffer);

        mFile = CreateFileA(fileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    }
}

FileOutputStream::~FileOutputStream()
{
    if (mFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(mFile);
    }
}

void FileOutputStream::write(unsigned int b)
{
    unsigned char value = (unsigned char)b;
    unsigned int bytesWritten;

    WriteFile(mFile, &value, 1, &bytesWritten, NULL);
}

void FileOutputStream::write(arrayWithLength<unsigned char> data)
{
    unsigned int bytesWritten;

    WriteFile(mFile, data.data, data.length, &bytesWritten, NULL);
}

void FileOutputStream::write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length)
{
    unsigned int bytesWritten;

    WriteFile(mFile, data.data + offset, length, &bytesWritten, NULL);
}

void FileOutputStream::close()
{
    CloseHandle(mFile);
    mFile = INVALID_HANDLE_VALUE;
}

void FileOutputStream::flush()
{
}
