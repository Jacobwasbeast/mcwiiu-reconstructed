#include "game/client/render/MobSkinTextureProcessor.hpp"

uint32_t MobSkinTextureProcessor::process(BufferedImage* image)
{
    return 0;
}

uint32_t MobSkinTextureProcessor::hasAlphaPixels(int x0, int y0, int x1, int y1)
{
    return (x0 < x1 && y0 < y1) ? 1u : 0u;
}

void MobSkinTextureProcessor::setForceAlpha(int x0, int y0, int x1, int y1)
{
    if (hasAlphaPixels(x0, y0, x1, y1) != 0)
        return;
}

void MobSkinTextureProcessor::setNoAlpha(int x0, int y0, int x1, int y1)
{
    if (x0 >= x1 || y0 >= y1)
        return;
    setForceAlpha(x0, y0, x1, y1);
}
