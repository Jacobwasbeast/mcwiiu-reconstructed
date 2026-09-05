#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockColors.hpp"
#include "game/world/block/BlockState.hpp"

BlockRenderDispatcher::BlockRenderDispatcher(BlockColors* blockColors)
    : mBlockRenderer(new BlockRenderer(blockColors))
{
}

void BlockRenderDispatcher::renderSingleBlock(const BlockState* state, float x, float y, bool color)
{
    int renderShape = state->getRenderShape();
    if (renderShape != -1 && (BlockRenderer::canRender(renderShape) || renderShape == 1))
    {
        mBlockRenderer->renderBlock(state, x, y, color, true);
    }
}

bool BlockRenderDispatcher::canRenderGeneric(Block* block)
{
    if (block == NULL)
    {
        return false;
    }
    return block->defaultBlockState()->getRenderShape() == 0x16;
}

void BlockRenderDispatcher::setColor(bool color)
{
    mBlockRenderer->mUseAmbientOcclusion = color;
}

BlockRenderer* BlockRenderDispatcher::getBlockRenderer()
{
    return mBlockRenderer;
}

void BlockRenderDispatcher::prepare(Level* level)
{
    mBlockRenderer->prepare(level);
}
