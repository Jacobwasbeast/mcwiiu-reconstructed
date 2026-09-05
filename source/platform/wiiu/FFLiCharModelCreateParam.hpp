#ifndef SOURCE_PLATFORM_WIIU_FFLICHARMODELCREATEPARAM_HPP
#define SOURCE_PLATFORM_WIIU_FFLICHARMODELCREATEPARAM_HPP

#include <stdint.h>

#include "platform/wiiu/FFLiCoordinate.hpp"

class FFLiDatabaseManager;
class FFLiResourceManager;
class FFLiShaderCallback;
class FFLiCharModelCreator;
struct FFLCharModelDesc;

enum FFLResolution
{
    FFL_RESOLUTION_512 = 512,
    FFL_RESOLUTION_1024 = 1024,
    FFL_RESOLUTION_2048 = 2048,
    FFL_RESOLUTION_4096 = 4096,
    FFL_RESOLUTION_MIPMAP_ENABLE = 0x40000000
};

class FFLiCharModelCreateParam
{
private:
    friend class FFLiCharModelCreator;

    FFLiDatabaseManager* m_databaseManager;
    FFLiResourceManager* m_resourceManager;
    FFLiShaderCallback* m_shaderCallback;
    FFLiCoordinate m_coordinate;
    bool m_useMipmap;

public:
    FFLiCharModelCreateParam(FFLiDatabaseManager* databaseManager,
                             FFLiResourceManager* resourceManager,
                             FFLiShaderCallback* shaderCallback,
                             bool useMipmap);
    ~FFLiCharModelCreateParam();
    static uint32_t GetResolution(FFLResolution resolution);
    static uint32_t IsEnabledMipMap(FFLResolution resolution);
    static uint32_t CheckModelDesc(const FFLCharModelDesc* description, bool specialMii);
    int32_t GetBufferSize(const FFLCharModelDesc* description) const;
    uint64_t GetCompressBufferSize(const FFLCharModelDesc* description) const;
    int32_t GetTempBufferSize(const FFLCharModelDesc* description) const;
    void SetCoordinate(FFLCoordinateType source, FFLCoordinateType destination);
    void SetScale(float scale);
};

#endif
