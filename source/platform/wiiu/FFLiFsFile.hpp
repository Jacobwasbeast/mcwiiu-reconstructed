#ifndef SOURCE_PLATFORM_WIIU_FFLIFSFILE_HPP
#define SOURCE_PLATFORM_WIIU_FFLIFSFILE_HPP

#include <stdint.h>
#include <coreinit/filesystem.h>
#include "platform/wiiu/FFLiFsCommand.hpp"

class FFLiFsFile
{
private:
    FFLiFsCommand *m_command;
    FSFileHandle m_handle;

public:
    FFLiFsFile(FFLiFsCommand *command);
    ~FFLiFsFile();
    int Open(const char *path, const char *mode, unsigned int error);
    int Close(unsigned int error);
    int Read(void *buffer, unsigned int count, unsigned int size, unsigned int error, unsigned int flags);
    int ReadWithPos(void *buffer, unsigned int count, unsigned int size, unsigned int position, unsigned int error, unsigned int flags);
    int Write(const void *buffer, unsigned int count, unsigned int size, unsigned int error, unsigned int flags);
    bool IsOpened() const;
    int GetPos(unsigned int *position, unsigned int error);
    int SetPos(unsigned int position, unsigned int error);
    int GetStat(FSStat *stat, unsigned int error);
    int WriteWithPos(const void *buffer, unsigned int count, unsigned int size, unsigned int position, unsigned int error, unsigned int flags);
    static int FlushQuota(FFLiFsCommand *command, const char *path, unsigned int error);
    static int Remove(FFLiFsCommand *command, const char *path, unsigned int error);
    static int Rename(FFLiFsCommand *command, const char *oldPath, const char *newPath, unsigned int error);
    static int GetStat(FSStat *stat, FFLiFsCommand *command, const char *path, unsigned int error);
};

#endif
