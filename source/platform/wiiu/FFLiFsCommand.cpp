#include "platform/wiiu/FFLiFsCommand.hpp"
#include <new>

extern "C" void FSInitCmdBlock(FFLiFsCommandBuffer *commandBlock);
extern "C" void FSCancel(FFLiFsClient *client, FFLiFsCommandBuffer *commandBlock);
extern "C" void FSSetUserData(FFLiFsCommandBuffer *commandBlock, void *userData);
extern "C" void *FSGetUserData(FFLiFsCommandBuffer *commandBlock);
extern "C" void FSCancelAll(FFLiFsClient *client);

FFLiFsCommand::FFLiFsCommand(FFLiFsClient *client)
    : m_client(client)
{
    FSInitCmdBlock(&m_commandBuffer);
}

FFLiFsCommand::~FFLiFsCommand()
{
}

void FFLiFsCommand::Cancel()
{
    FSCancel(m_client, &m_commandBuffer);
}

void FFLiFsCommand::SetUserData(void *userData)
{
    FSSetUserData(&m_commandBuffer, userData);
}

void *FFLiFsCommand::GetUserData()
{
    return FSGetUserData(&m_commandBuffer);
}

void FFLiFsCommand::CancelAll(FFLiFsClient *client)
{
    FSCancelAll(client);
}

FFLiFsCommand *FFLiFsCommand::PlacementNew(FFLiFsCommandBuffer *buffer, FFLiFsClient *client)
{
    if (buffer == 0)
    {
        return 0;
    }
    return new (buffer) FFLiFsCommand(client);
}

void FFLiFsCommand::PlacementDelete(FFLiFsCommand *command)
{
    if (command != 0)
    {
        command->~FFLiFsCommand();
    }
}
