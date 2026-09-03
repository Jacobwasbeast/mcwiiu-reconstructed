#ifndef PLATFORM_WIIU_FFLICOPYSURFACE_HPP
#define PLATFORM_WIIU_FFLICOPYSURFACE_HPP

#include "platform/wiiu/FFLiCopySurfaceShader.hpp"
#include "platform/wiiu/FFLiCopySurfaceDrawer.hpp"

class FFLiBufferAllocator;
struct _GX2Surface;

class FFLiCopySurface : public FFLiCopySurfaceShader
{
public:
    FFLiCopySurface(FFLiBufferAllocator* allocator);
    ~FFLiCopySurface();

    static unsigned int GetBufferSize();
    void SetupGPU();
    void Begin();
    void SetupSrcSurface(const _GX2Surface* surface, unsigned int level);
    void SetupDstSurface(_GX2Surface* surface, unsigned int level);
    void Execute(_GX2Surface* destination, unsigned int destinationLevel,
                 const _GX2Surface* source, unsigned int sourceLevel);
    bool CanInitCharModel(bool first, bool second) const;

private:
    FFLiCopySurfaceDrawer mDrawer;
};

#endif
