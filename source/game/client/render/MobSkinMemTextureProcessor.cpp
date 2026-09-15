#include "game/client/render/MobSkinMemTextureProcessor.hpp"
#include "game/core/BufferedImage.hpp"

MobSkinMemTextureProcessor::~MobSkinMemTextureProcessor()
{
}

void MobSkinMemTextureProcessor::setNoAlpha(int x0, int y0, int x1, int y1)
{
    int y;
    for (y = y0; y < y1; ++y)
    {
        int x;
        for (x = x0; x < x1; ++x)
            m_data[x + y * m_width] |= (int)0xff000000u;
    }
}

uint32_t MobSkinMemTextureProcessor::hasAlpha(int x0, int y0, int x1, int y1)
{
    int y;
    for (y = y0; y < y1; ++y)
    {
        int x;
        for (x = x0; x < x1; ++x)
        {
            if (((unsigned int)m_data[x + y * m_width] >> 24) < 0x80u)
                return 1;
        }
    }
    return 0;
}

void MobSkinMemTextureProcessor::setForceAlpha(int x0, int y0, int x1, int y1)
{
    if (hasAlpha(x0, y0, x1, y1) == 0)
    {
        int y;
        for (y = y0; y < y1; ++y)
        {
            int x;
            for (x = x0; x < x1; ++x)
                m_data[x + y * m_width] &= (int)0x00ffffffu;
        }
    }
}

BufferedImage* MobSkinMemTextureProcessor::process(BufferedImage* image)
{
    if (image == NULL)
        return NULL;
    m_height = 0x20;
    m_width = 0x40;
    BufferedImage* result = new BufferedImage(m_width, m_height, 1);
    result->getGraphics();
    m_data = result->getData();
    setNoAlpha(0, 0, 0x20, 0x10);
    setForceAlpha(0x20, 0, 0x40, 0x20);
    setNoAlpha(0, 0x10, 0x40, 0x20);
    return result;
}
