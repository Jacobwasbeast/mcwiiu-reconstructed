#ifndef PLATFORM_WIIU_FFLICOPYSURFACEDRAWER_HPP
#define PLATFORM_WIIU_FFLICOPYSURFACEDRAWER_HPP

class FFLiBufferAllocator;

class FFLiCopySurfaceDrawer
{
public:
    FFLiCopySurfaceDrawer();
    ~FFLiCopySurfaceDrawer();
    void SetupCPU(FFLiBufferAllocator* allocator);
    void SetupGPU();
    void SetAttributeBuffer(unsigned int index, unsigned int bufferIndex);
    void Draw();
    static unsigned int GetBufferSize();

private:
    unsigned int mPositionBuffer;
    unsigned int mTexCoordBuffer;
};

#endif
