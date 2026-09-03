#ifndef PLATFORM_WIIU_FFLICOMPRESSDRAWER_HPP
#define PLATFORM_WIIU_FFLICOMPRESSDRAWER_HPP

class FFLiBufferAllocator;

class FFLiCompressDrawer
{
public:
    FFLiCompressDrawer();
    ~FFLiCompressDrawer();
    void Setup(FFLiBufferAllocator* allocator);
    void SetupCPU(FFLiBufferAllocator* allocator);
    void SetupGPU();
    void SetAttributeBuffer(unsigned int index, unsigned int bufferIndex);
    void Draw();
    static unsigned int GetBufferSize();

private:
    unsigned int mVertexBuffer;
    unsigned int mTexCoordBuffer;
};

#endif
