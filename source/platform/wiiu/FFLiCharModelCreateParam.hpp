#ifndef SOURCE_PLATFORM_WIIU_FFLICHARMODELCREATEPARAM_HPP
#define SOURCE_PLATFORM_WIIU_FFLICHARMODELCREATEPARAM_HPP

#include <stdint.h>

#include "platform/wiiu/FFLiCoordinate.hpp"

class FFLiDatabaseManager;
class FFLiResourceManager;
class FFLiShaderCallback;
struct FFLCharModelDesc;

class FFLiCharModelCreateParam
{
private:
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
    static uint32_t GetResolution(uint32_t resolution);
    static uint32_t IsEnabledMipMap(uint32_t resolution);
    static uint32_t CheckModelDesc(const FFLCharModelDesc* description, bool specialMii);
    int32_t GetBufferSize(const FFLCharModelDesc* description) const;
    uint32_t GetCompressBufferSize(const FFLCharModelDesc* description) const;
    int32_t GetTempBufferSize(const FFLCharModelDesc* description) const;
    void SetCoordinate(FFLCoordinateType source, FFLCoordinateType destination);
    void SetScale(float scale);
};

#endif
