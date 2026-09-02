#include "game/client/render/OffsettedRenderList.hpp"

#include "game/core/GlStateManager.hpp"
#include "game/core/IntBuffer.hpp"

void OffsettedRenderList::render()
{
    if (!valid)
    {
        return;
    }

    if (!bufferUploaded)
    {
        glListBuffer->flip();
        bufferUploaded = true;
    }

    if (glListBuffer->remaining() > 0)
    {
        GlStateManager::pushMatrix();
        GlStateManager::translatef((float)x - cameraX, (float)y - cameraY, (float)z - cameraZ);
        GlStateManager::callLists(glListBuffer);
        GlStateManager::popMatrix();
    }
}
