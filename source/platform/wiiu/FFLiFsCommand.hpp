#ifndef SOURCE_PLATFORM_WIIU_FFLIFSCOMMAND_HPP
#define SOURCE_PLATFORM_WIIU_FFLIFSCOMMAND_HPP

#include <stdint.h>
#include "platform/wiiu/FFLiFsClient.hpp"

struct FFLiFsCommandBuffer
{
    uint8_t commandData[2688];
};

class FFLiFsFile;

class FFLiFsCommand
{
private:
    friend class FFLiFsFile;
    FFLiFsCommandBuffer m_commandBuffer;
    FFLiFsClient *m_client;

public:
    FFLiFsCommand(FFLiFsClient *client);
    ~FFLiFsCommand();
    void Cancel();
    void SetUserData(void *userData);
    void *GetUserData();
    static void CancelAll(FFLiFsClient *client);
    static FFLiFsCommand *PlacementNew(FFLiFsCommandBuffer *buffer, FFLiFsClient *client);
    static void PlacementDelete(FFLiFsCommand *command);
};

#endif
