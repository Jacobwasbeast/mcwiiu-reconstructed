#include "platform/wiiu/FFLiManager.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiCompressor.hpp"
#include "platform/wiiu/FFLiFsCommand.hpp"
#include "vendor/__N_15_FFLiManager_cpp_28866420.hpp"

#include <new>

static FFLiManager* s_fflManager = 0;

static unsigned int AllocateOptional(FFLiBufferAllocator* allocator)
{
    if (allocator->CanAllocate(0x40000, 0x40))
    {
        return allocator->Allocate(0x40000, 0x40);
    }
    return 0;
}

FFLiManager::FFLiManager(const FFLInitDesc* initDesc, FFLiBufferAllocator* allocator)
    : FFLiAllocator(), m_systemContext(),
      m_resourceHeader((FFLiResourceMultiHeader*)allocator->Allocate(0x9400, 0x40)),
      m_databaseFile((FFLiDatabaseFile*)allocator->Allocate(0x4e440, 0x40)),
      m_fileWriteBuffer((FFLiFileWriteBuffer*)(uintptr_t)AllocateOptional(allocator)),
      m_fsClient(),
      m_resourceManager(m_resourceHeader, &m_fsClient),
      m_databaseManager(m_databaseFile, m_fileWriteBuffer,
                        &m_systemContext, &m_fsClient, this),
      m_shaderCallback(),
      m_charModelCreateParam(&m_databaseManager, &m_resourceManager, &m_shaderCallback,
                             initDesc->resource != 0),
      m_compressor(__N_15_FFLiManager_cpp_28866420::CreateCompressor(initDesc->resource, allocator)),
      m_copySurface(allocator), m_gpuReady(false), m_unknownFlag(false)
{
    unsigned int i;
    const uint32_t* values = (const uint32_t*)initDesc;
    uint32_t* copiedInitDesc = (uint32_t*)&m_initDesc;
    for (i = 0; i < 5; ++i)
    {
        copiedInitDesc[i] = values[i];
    }
    unsigned int restSize = allocator->GetRestSize();
    Init((void*)(uintptr_t)allocator->Allocate(restSize, 1), restSize);
}

FFLiManager::~FFLiManager()
{
}

bool FFLiManager::IsConstruct()
{
    return s_fflManager != 0;
}

int32_t FFLiManager::AfterConstruct(const FFLInitDesc* initDesc, const FFLResourceDesc* resourceDesc)
{
    int32_t result = (int32_t)m_systemContext.AfterConstruct();
    if (result == 0)
    {
        return 0x11;
    }
    result = m_fsClient.Init(initDesc->stateChangeParams, 0);
    if (result != 0)
    {
        return 0x12;
    }
    if (m_compressor != 0 && !m_compressor->SetupCPU(initDesc->resource))
    {
        return 1;
    }
    result = (int32_t)m_resourceManager.AfterConstruct();
    if (result == 0)
    {
        if (resourceDesc == 0)
        {
            FFLiFsCommandBuffer* buffer = (FFLiFsCommandBuffer*)Allocate(0xa84, 4);
            result = (int32_t)m_resourceManager.LoadResourceHeader(buffer);
            Free(buffer);
            if (result != 0)
            {
                m_fsClient.Shutdown(0);
                return result;
            }
        }
        else
        {
            unsigned int i;
            for (i = 0; i < 2; ++i)
            {
                if (resourceDesc->data[i] != 0)
                {
                    result = (int32_t)m_resourceManager.AttachCache(resourceDesc->data[i],
                                                                    resourceDesc->size[i],
                                                                    (FFLResourceType)i);
                    if (result != 0)
                    {
                        m_fsClient.Shutdown(0);
                        return result;
                    }
                }
            }
        }
        result = (int32_t)m_databaseManager.AfterConstruct();
    }
    return result;
}

int32_t FFLiManager::GetBufferSize(const FFLInitDesc* initDesc)
{
    int32_t size;
    if (initDesc == 0)
    {
        return 0;
    }
    size = initDesc->resource != 0 ? FFLiCompressor::GetBufferSize(initDesc->resource) + 0x9a6b8 : 0x9a4c8;
    return size + (int32_t)FFLiCopySurface::GetBufferSize() +
           (int32_t)__N_15_FFLiManager_cpp_28866420::TempMemorySize(initDesc->temporary != 0);
}

FFLiManager* FFLiManager::GetInstance()
{
    return s_fflManager;
}

int32_t FFLiManager::BeforeDestruct()
{
    int32_t result = (int32_t)m_databaseManager.BeforeDestruct();
    if (m_fsClient.ShutdownIfValid(0) != 0 && (result == 0 || result == 10))
    {
        result = 0x12;
    }
    return result;
}

void FFLiManager::FlushQuota(bool force)
{
    m_databaseManager.FlushQuota(force);
}

uint32_t FFLiManager::Destroy()
{
    int32_t result;
    if (!IsConstruct())
    {
        return 4;
    }
    s_fflManager->FlushQuota(false);
    result = s_fflManager->BeforeDestruct();
    s_fflManager->~FFLiManager();
    s_fflManager = 0;
    return (uint32_t)result;
}

int32_t FFLiManager::Create(void* work, const FFLInitDesc* initDesc, const FFLResourceDesc* resourceDesc)
{
    FFLiBufferAllocator allocator;
    if (IsConstruct())
    {
        return 0;
    }
    if (initDesc == 0)
    {
        return 1;
    }
    allocator.Init(work, (unsigned int)GetBufferSize(initDesc));
    FFLiManager* manager = (FFLiManager*)allocator.Allocate(sizeof(FFLiManager), 4);
    if (manager != 0)
    {
        manager = new (manager) FFLiManager(initDesc, &allocator);
    }
    s_fflManager = manager;
    int32_t result = manager->AfterConstruct(initDesc, resourceDesc);
    if (result != 0 && result != 10)
    {
        Destroy();
    }
    return result;
}

void FFLiManager::SetupGPU()
{
    m_gpuReady = true;
    m_copySurface.SetupGPU();
    if (m_compressor != 0)
    {
        m_compressor->SetupGPU();
    }
}

void FFLiManager::CanInitCharModel() const
{
    m_copySurface.CanInitCharModel(m_gpuReady, m_compressor != 0);
}
