#include "game/client/render/FallingBlockRenderer.hpp"

#include <cmath>

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FallingBlockEntity.hpp"

static ResourceLocation FALLING_BLOCK_LOCATION;

FallingBlockRenderer::FallingBlockRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
    mShadowRadius = 0.5F;
}

FallingBlockRenderer::~FallingBlockRenderer()
{
}

void FallingBlockRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                  float yaw, float partialTick)
{
    boost::shared_ptr<FallingBlockEntity> fallingBlock = boost::static_pointer_cast<FallingBlockEntity>(entity);
    const BlockState* blockState = fallingBlock->getBlock();
    if (blockState == NULL)
    {
        EntityRenderer::render(entity, x, y, z, yaw, partialTick);
        return;
    }

    bindTexture(getTextureLocation(entity));

    BlockPos blockPos(fallingBlock);
    Level* level = fallingBlock->getLevel();
    const BlockState* levelState = level->getBlockState(blockPos);
    if (blockState != levelState)
    {
        Block* block = blockState->getBlock();
        const BlockState* defaultState = block->defaultBlockState();
        if (defaultState->getRenderShape() != -1)
        {
            GlStateManager::pushMatrix();
            GlStateManager::translatef((float)x, (float)y, (float)z);
            GlStateManager::disableLighting();
            GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

            BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
            BlockRenderer* blockRenderer = Minecraft::GetInstance()->getBlockRenderer()->getBlockRenderer();
            blockRenderer->prepare(level);

            float offset = 0.5F;
            if (block == Blocks::anvil && defaultState->getRenderShape() == 0x23)
            {
                builder->begin();
                builder->offset((float)(std::floor(fallingBlock->mX) - fallingBlock->mX - offset),
                                (float)(std::floor(fallingBlock->mY) - fallingBlock->mY),
                                (float)(std::floor(fallingBlock->mZ) - fallingBlock->mZ - offset));
                blockRenderer->tesselateAnvilInWorld(blockState, blockPos);
                builder->offset(0.0F, 0.0F, 0.0F);
                builder->end();
            }
            else
            {
                builder->begin();
                builder->offset((float)(std::floor(fallingBlock->mX) - fallingBlock->mX - offset),
                                (float)(std::floor(fallingBlock->mY) - fallingBlock->mY),
                                (float)(std::floor(fallingBlock->mZ) - fallingBlock->mZ - offset));
                boost::shared_ptr<BlockEntity> blockEntity;
                blockRenderer->tesselateInWorldNoCulling(blockState, blockPos, NULL, blockEntity);
                builder->offset(0.0F, 0.0F, 0.0F);
                builder->end();
            }

            GlStateManager::enableLighting();
            GlStateManager::popMatrix();
        }
    }

    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* FallingBlockRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &FALLING_BLOCK_LOCATION;
}
