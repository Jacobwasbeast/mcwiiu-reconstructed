#include "platform/wiiu/FFLiFsFile.hpp"

extern "C" int FSOpenFile(FFLiFsClient *, FFLiFsCommandBuffer *, const char *, const char *, int *, unsigned int);
extern "C" int FSCloseFile(FFLiFsClient *, FFLiFsCommandBuffer *, int, unsigned int);
extern "C" int FSReadFile(FFLiFsClient *, FFLiFsCommandBuffer *, void *, unsigned int, unsigned int, int, unsigned int, unsigned int);
extern "C" int FSReadFileWithPos(FFLiFsClient *, FFLiFsCommandBuffer *, void *, unsigned int, unsigned int, unsigned int, int, unsigned int, unsigned int);
extern "C" int FSWriteFile(FFLiFsClient *, FFLiFsCommandBuffer *, const void *, unsigned int, unsigned int, int, unsigned int, unsigned int);
extern "C" int FSWriteFileWithPos(FFLiFsClient *, FFLiFsCommandBuffer *, const void *, unsigned int, unsigned int, unsigned int, int, unsigned int, unsigned int);
extern "C" int FSGetPosFile(FFLiFsClient *, FFLiFsCommandBuffer *, int, unsigned int *, unsigned int);
extern "C" int FSSetPosFile(FFLiFsClient *, FFLiFsCommandBuffer *, int, unsigned int, unsigned int);
extern "C" int FSGetStatFile(FFLiFsClient *, FFLiFsCommandBuffer *, FSStat *, int, unsigned int);
extern "C" int FSFlushQuota(FFLiFsClient *, FFLiFsCommandBuffer *, const char *, unsigned int);
extern "C" int FSRemove(FFLiFsClient *, FFLiFsCommandBuffer *, const char *, unsigned int);
extern "C" int FSRename(FFLiFsClient *, FFLiFsCommandBuffer *, const char *, const char *, unsigned int);
extern "C" int FSGetStat(FFLiFsClient *, FFLiFsCommandBuffer *, const char *, FSStat *, unsigned int);

FFLiFsFile::FFLiFsFile(FFLiFsCommand *command)
    : m_command(command), m_handle(-1)
{
}

FFLiFsFile::~FFLiFsFile()
{
}

int FFLiFsFile::Open(const char *path, const char *mode, unsigned int error)
{
    return FSOpenFile(m_command->m_client, &m_command->m_commandBuffer, path, mode, &m_handle, error);
}

int FFLiFsFile::Close(unsigned int error)
{
    int result = FSCloseFile(m_command->m_client, &m_command->m_commandBuffer, m_handle, error);
    if (result == 0)
    {
        m_handle = -1;
    }
    return result;
}

int FFLiFsFile::Read(void *buffer, unsigned int count, unsigned int size, unsigned int error, unsigned int flags)
{
    return FSReadFile(m_command->m_client, &m_command->m_commandBuffer, buffer, count, size, m_handle, flags, error);
}

int FFLiFsFile::ReadWithPos(void *buffer, unsigned int count, unsigned int size, unsigned int position, unsigned int error, unsigned int flags)
{
    return FSReadFileWithPos(m_command->m_client, &m_command->m_commandBuffer, buffer, count, size, position, m_handle, flags, error);
}

int FFLiFsFile::Write(const void *buffer, unsigned int count, unsigned int size, unsigned int error, unsigned int flags)
{
    return FSWriteFile(m_command->m_client, &m_command->m_commandBuffer, buffer, count, size, m_handle, flags, error);
}

bool FFLiFsFile::IsOpened() const
{
    return m_handle != -1;
}

int FFLiFsFile::GetPos(unsigned int *position, unsigned int error)
{
    return FSGetPosFile(m_command->m_client, &m_command->m_commandBuffer, m_handle, position, error);
}

int FFLiFsFile::SetPos(unsigned int position, unsigned int error)
{
    return FSSetPosFile(m_command->m_client, &m_command->m_commandBuffer, m_handle, position, error);
}

int FFLiFsFile::GetStat(FSStat *stat, unsigned int error)
{
    return FSGetStatFile(m_command->m_client, &m_command->m_commandBuffer, stat, m_handle, error);
}

int FFLiFsFile::WriteWithPos(const void *buffer, unsigned int count, unsigned int size, unsigned int position, unsigned int error, unsigned int flags)
{
    return FSWriteFileWithPos(m_command->m_client, &m_command->m_commandBuffer, buffer, count, size, position, m_handle, flags, error);
}

int FFLiFsFile::FlushQuota(FFLiFsCommand *command, const char *path, unsigned int error)
{
    return FSFlushQuota(command->m_client, &command->m_commandBuffer, path, error);
}

int FFLiFsFile::Remove(FFLiFsCommand *command, const char *path, unsigned int error)
{
    return FSRemove(command->m_client, &command->m_commandBuffer, path, error);
}

int FFLiFsFile::Rename(FFLiFsCommand *command, const char *oldPath, const char *newPath, unsigned int error)
{
    return FSRename(command->m_client, &command->m_commandBuffer, oldPath, newPath, error);
}

int FFLiFsFile::GetStat(FSStat *stat, FFLiFsCommand *command, const char *path, unsigned int error)
{
    return FSGetStat(command->m_client, &command->m_commandBuffer, path, stat, error);
}
