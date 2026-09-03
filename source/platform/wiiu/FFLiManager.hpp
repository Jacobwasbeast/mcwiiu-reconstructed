#ifndef PLATFORM_WIIU_FFLIMANAGER_HPP
#define PLATFORM_WIIU_FFLIMANAGER_HPP

#include <stdint.h>

#include "platform/wiiu/FFLiAllocator.hpp"
#include "platform/wiiu/FFLiSystemContext.hpp"
#include "platform/wiiu/FFLiFsClient.hpp"
#include "platform/wiiu/FFLiResourceManager.hpp"
#include "platform/wiiu/FFLiDatabaseManager.hpp"
#include "platform/wiiu/FFLiShaderCallback.hpp"
#include "platform/wiiu/FFLiCharModelCreateParam.hpp"
#include "platform/wiiu/FFLiCopySurface.hpp"

struct FFLInitDesc
{
    const void* resource;
    const FSStateChangeParams* stateChangeParams;
    uint32_t resourceConfig;
    uint32_t fileSystemConfig;
    uint32_t temporary;
};

struct FFLResourceDesc
{
    void* data[2];
    uint32_t size[2];
};

class FFLiBufferAllocator;
class FFLiCompressor;

class FFLiManager : public FFLiAllocator
{
private:
    FFLiSystemContext m_systemContext;
    unsigned char m_fsPadding[3];
    FFLiFsClient m_fsClient;
    FFLiResourceManager m_resourceManager;
    FFLiDatabaseManager m_databaseManager;
    FFLiShaderCallback m_shaderCallback;
    FFLiCharModelCreateParam m_charModelCreateParam;
    FFLInitDesc m_initDesc;
    FFLiCompressor* m_compressor;
    FFLiCopySurface m_copySurface;
    bool m_gpuReady;
    bool m_hasCompressor;

public:
    FFLiManager(const FFLInitDesc* initDesc, FFLiBufferAllocator* allocator);
    ~FFLiManager();
    static bool IsConstruct();
    int32_t AfterConstruct(const FFLInitDesc* initDesc, const FFLResourceDesc* resourceDesc);
    static int32_t GetBufferSize(const FFLInitDesc* initDesc);
    static FFLiManager* GetInstance();
    int32_t BeforeDestruct();
    void FlushQuota(bool force);
    static uint32_t Destroy();
    static int32_t Create(void* work, const FFLInitDesc* initDesc, const FFLResourceDesc* resourceDesc);
    void SetupGPU();
    void CanInitCharModel() const;
};

#endif
