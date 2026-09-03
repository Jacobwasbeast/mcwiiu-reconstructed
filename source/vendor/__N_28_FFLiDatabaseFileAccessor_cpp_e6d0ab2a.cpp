#include "vendor/__N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a.hpp"
#include "platform/wiiu/FFLiAllocator.hpp"
#include "platform/wiiu/FFLiFsCommand.hpp"
#include "platform/wiiu/FFLiFsFile.hpp"

#include <cstring>

extern int FFLiGetDatabasePath(char *path, unsigned int pathSize, unsigned long long titleId, const char *relativePath);
extern unsigned int FFLiConvertFSStatusToFFLResult(int status);
extern unsigned int FFLiMin(unsigned int first, unsigned int second);
extern int FFLiMakeDatabaseDir(FFLiFsCommand *command, const char *path);

int __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::GetDatabasePath(char *path, unsigned int pathSize, unsigned long long titleId, const char *relativePath)
{
    return static_cast<int>(FFLiConvertFSStatusToFFLResult(FFLiGetDatabasePath(path, pathSize, titleId, relativePath)));
}

int __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::ReadFile(void *data, unsigned int size, FFLiFsCommand *command, const char *path)
{
    FFLiFsFile file(command);
    int result = file.Open(path, "r", 8);
    if (result != 0)
        return result;
    result = file.Read(data, size, 1, 0, 0);
    if (result < 0)
    {
        file.Close(0);
        return result;
    }
    if (result < 1)
    {
        file.Close(0);
        return 1;
    }
    result = file.Close(0);
    if (result != 0)
        return result;
    return 0;
}

int __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::LoadDatabaseHidden(FFLiDatabaseFileHidden *database, FFLiFsCommand *command, const char *path)
{
    return ReadFile(database, 0xac00, command, path);
}

int __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::WriteFileImpl(FFLiFsFile &file, const void *data, unsigned int size, FFLiFileWriteBuffer *writeBuffer)
{
    unsigned int blocks = (size + 0x3ffff) >> 18;
    unsigned int offset = 0;
    int result = 0;
    while (offset < blocks)
    {
        unsigned int chunk = FFLiMin(size - offset * 0x40000, 0x40000);
        std::memcpy(writeBuffer, static_cast<const unsigned char *>(data) + offset * 0x40000, chunk);
        result = file.Write(writeBuffer, chunk, 1, 0, 0);
        if (result != 1)
            return result;
        ++offset;
    }
    return result;
}

int __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::WriteFile(const void *data, unsigned int size, FFLiFsCommand *command, FFLiFileWriteBuffer *writeBuffer, const char *path)
{
    FFLiFsFile file(command);
    int result = file.Open(path, "w", 0x88);
    if (result != 0)
    {
        if (result == -6 || result == -10)
            return 4;
        return result;
    }
    result = WriteFileImpl(file, data, size, writeBuffer);
    if (result < 0)
    {
        file.Close(0);
        return result;
    }
    if (result < 1)
    {
        file.Close(0);
        return 2;
    }
    result = file.Close(0);
    if (result != 0)
        return result;
    return 0;
}

int __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::SaveDatabaseHidden(const FFLiDatabaseFileHidden &database, FFLiFsCommand *command, FFLiFileWriteBuffer *writeBuffer, const char *path)
{
    return WriteFile(&database, 0xac00, command, writeBuffer, path);
}

int __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::LoadDatabaseOfficial(FFLiDatabaseFileOfficial *database, FFLiFsCommand *command, const char *path)
{
    return ReadFile(database, 0x43840, command, path);
}

int __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::SaveDatabaseOfficial(const FFLiDatabaseFileOfficial &database, FFLiFsCommand *command, FFLiFileWriteBuffer *writeBuffer, const char *path)
{
    return WriteFile(&database, 0x43840, command, writeBuffer, path);
}

int __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::CopyDatabaseOfficial(const char *destinationPath, const char *sourcePath, FFLiFsCommand *command, FFLiFileWriteBuffer *writeBuffer, FFLiAllocator *allocator)
{
    FFLiDatabaseFileOfficial *database = static_cast<FFLiDatabaseFileOfficial *>(allocator->Allocate(0x43840, 0x40));
    if (database == 0)
        return 3;
    int result = LoadDatabaseOfficial(database, command, sourcePath);
    if (result == 0)
        result = SaveDatabaseOfficial(*database, command, writeBuffer, destinationPath);
    allocator->Free(database);
    return result;
}

int __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::GetDirPath(char *directoryPath, const char *path)
{
    const char *separator = std::strrchr(path, '/');
    if (separator == 0)
        separator = std::strrchr(path, '\\');
    if (separator == 0)
    {
        directoryPath[0] = 0;
        return 0;
    }
    std::memcpy(directoryPath, path, static_cast<unsigned int>(separator - path));
    directoryPath[separator - path] = 0;
    return 0;
}

int __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a::MakeDatabaseDir(FFLiFsCommand *command, const char *path)
{
    return FFLiMakeDatabaseDir(command, path);
}
