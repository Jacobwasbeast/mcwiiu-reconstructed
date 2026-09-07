#include "platform/wiiu/FFLiFsFile.hpp"

extern "C" FSStatus FSFlushQuota(FSClient *, FSCmdBlock *, const char *, FSErrorFlag);

static FSClient *GetFSClient(FFLiFsClient *client)
{
    return reinterpret_cast<FSClient *>(client);
}

static FSCmdBlock *GetFSCmdBlock(FFLiFsCommandBuffer *commandBuffer)
{
    return reinterpret_cast<FSCmdBlock *>(commandBuffer);
}

FFLiFsFile::FFLiFsFile(FFLiFsCommand *command)
    : m_command(command), m_handle(-1)
{
}

FFLiFsFile::~FFLiFsFile()
{
}

int FFLiFsFile::Open(const char *path, const char *mode, unsigned int error)
{
    return FSOpenFile(GetFSClient(m_command->m_client), GetFSCmdBlock(&m_command->m_commandBuffer), path, mode,
                      &m_handle, static_cast<FSErrorFlag>(error));
}

int FFLiFsFile::Close(unsigned int error)
{
    int result = FSCloseFile(GetFSClient(m_command->m_client), GetFSCmdBlock(&m_command->m_commandBuffer), m_handle,
                             static_cast<FSErrorFlag>(error));
    if (result == 0)
    {
        m_handle = -1;
    }
    return result;
}

int FFLiFsFile::Read(void *buffer, unsigned int count, unsigned int size, unsigned int error, unsigned int flags)
{
    return FSReadFile(GetFSClient(m_command->m_client), GetFSCmdBlock(&m_command->m_commandBuffer),
                      static_cast<uint8_t *>(buffer), count, size, m_handle, flags,
                      static_cast<FSErrorFlag>(error));
}

int FFLiFsFile::ReadWithPos(void *buffer, unsigned int count, unsigned int size, unsigned int position, unsigned int error, unsigned int flags)
{
    return FSReadFileWithPos(GetFSClient(m_command->m_client), GetFSCmdBlock(&m_command->m_commandBuffer),
                             static_cast<uint8_t *>(buffer), count, size, position, m_handle, flags,
                             static_cast<FSErrorFlag>(error));
}

int FFLiFsFile::Write(const void *buffer, unsigned int count, unsigned int size, unsigned int error, unsigned int flags)
{
    return FSWriteFile(GetFSClient(m_command->m_client), GetFSCmdBlock(&m_command->m_commandBuffer),
                       const_cast<uint8_t *>(static_cast<const uint8_t *>(buffer)), count, size, m_handle, flags,
                       static_cast<FSErrorFlag>(error));
}

bool FFLiFsFile::IsOpened() const
{
    return m_handle != -1;
}

int FFLiFsFile::FlushQuota(FFLiFsCommand *command, const char *path, unsigned int error)
{
    return FSFlushQuota(GetFSClient(command->m_client), GetFSCmdBlock(&command->m_commandBuffer), path,
                        static_cast<FSErrorFlag>(error));
}
