#ifndef PLATFORM_WIIU_FFLICHARMODELCREATOR_HPP
#define PLATFORM_WIIU_FFLICHARMODELCREATOR_HPP

class FFLiCharModelCreateParam;
class FFLiManager;
class FFLiBufferAllocator;
struct FFLShaderCallback;
struct FFLiCharModel;
struct FFLCharModelSource;
struct FFLCharModelDesc;
struct FFLCharModelBuffer;
struct FFLiRenderTextureBuffer;

class FFLiCharModelCreator
{
private:
    FFLiCharModelCreateParam* m_createParam;
    FFLiManager* m_manager;

public:
    FFLiCharModelCreator(FFLiCharModelCreateParam* createParam, FFLiManager* manager);
    ~FFLiCharModelCreator();
    void SetupRenderTextureBuffer(FFLiRenderTextureBuffer& buffer, const FFLCharModelDesc* description, FFLiBufferAllocator* allocator) const;
    int ExecuteCPUStep(FFLiCharModel* model, const FFLCharModelSource* source, const FFLCharModelDesc* description, FFLCharModelBuffer* buffer);
    void InvalidateShapes(FFLiCharModel* model);
    void BeforeExecuteGPUStep(FFLiCharModel* model);
    void AfterExecuteGPUStep(FFLiCharModel* model);
    void ExecuteGPUStep(FFLiCharModel* model, const FFLShaderCallback* callback);
};

#endif
