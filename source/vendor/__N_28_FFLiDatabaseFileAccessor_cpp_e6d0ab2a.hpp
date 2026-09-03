#ifndef SOURCE_VENDOR_N_28_FFLIDATABASEFILEACCESSOR_CPP_E6D0AB2A_HPP
#define SOURCE_VENDOR_N_28_FFLIDATABASEFILEACCESSOR_CPP_E6D0AB2A_HPP

class FFLiAllocator;
class FFLiDatabaseFileHidden;
class FFLiDatabaseFileOfficial;
class FFLiFsCommand;
class FFLiFsFile;
class FFLiFileWriteBuffer;

class __N_28_FFLiDatabaseFileAccessor_cpp_e6d0ab2a
{
public:
    static int GetDatabasePath(char *path, unsigned int pathSize, unsigned long long titleId, const char *relativePath);
    static int ReadFile(void *data, unsigned int size, FFLiFsCommand *command, const char *path);
    static int LoadDatabaseHidden(FFLiDatabaseFileHidden *database, FFLiFsCommand *command, const char *path);
    static int WriteFileImpl(FFLiFsFile &file, const void *data, unsigned int size, FFLiFileWriteBuffer *writeBuffer);
    static int WriteFile(const void *data, unsigned int size, FFLiFsCommand *command, FFLiFileWriteBuffer *writeBuffer, const char *path);
    static int SaveDatabaseHidden(const FFLiDatabaseFileHidden &database, FFLiFsCommand *command, FFLiFileWriteBuffer *writeBuffer, const char *path);
    static int LoadDatabaseOfficial(FFLiDatabaseFileOfficial *database, FFLiFsCommand *command, const char *path);
    static int SaveDatabaseOfficial(const FFLiDatabaseFileOfficial &database, FFLiFsCommand *command, FFLiFileWriteBuffer *writeBuffer, const char *path);
    static int CopyDatabaseOfficial(const char *destinationPath, const char *sourcePath, FFLiFsCommand *command, FFLiFileWriteBuffer *writeBuffer, FFLiAllocator *allocator);
    static int GetDirPath(char *directoryPath, const char *path);
    static int MakeDatabaseDir(FFLiFsCommand *command, const char *path);
};

#endif
