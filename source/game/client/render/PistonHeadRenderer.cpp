#include "game/client/render/PistonHeadRenderer.hpp"

#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/DefaultVertexFormat.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/world/Level.hpp"
#include "game/core/Boxed.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/AbstractProperty.hpp"

static ResourceLocation sPistonTexture;

PistonHeadRenderer::PistonHeadRenderer()
{
    mBlockRenderer = NULL;
}

PistonHeadRenderer::~PistonHeadRenderer()
{
}

void PistonHeadRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                                float partialTick, int breakProgress, bool bTranslucent, float alpha,
                                bool bUseDisplayList)
{
    boost::shared_ptr<PistonMovingBlockEntity> movingBlock =
        boost::static_pointer_cast<PistonMovingBlockEntity>(blockEntity);
    const BlockState* movedState = movingBlock->getBlockState();
    if (movedState == NULL || movedState->getBlock() == Blocks::AIR)
        return;

    const Block* movedBlock = movedState->getBlock();
    if (movedState->getRenderShape() == BLOCK_RENDER_INVISIBLE)
        return;

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    bindTexture(&sPistonTexture);
    Lighting::turnOff();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::enableBlend();
    GlStateManager::disableCull();
    Minecraft::useAmbientOcclusion();
    GlStateManager::shadeModel(0);
    builder->begin();
    builder->setVertexFormat(DefaultVertexFormat::BLOCK);

    BlockPos position = movingBlock->getBlockPos();
    const double xOffset = movingBlock->getXOff(partialTick);
    const double yOffset = movingBlock->getYOff(partialTick);
    const double zOffset = movingBlock->getZOff(partialTick);
    builder->offset((float)((float)x - (float)position.x + xOffset),
                    (float)((float)y - (float)position.y + yOffset),
                    (float)((float)z - (float)position.z + zOffset));
    builder->color(1.0F, 1.0F, 1.0F);

    if (movedBlock == Blocks::piston && movingBlock->getProgress(partialTick) < 0.25)
    {
        bool shortHead = true;
        TypedBoxed<bool> shortHeadValue(shortHead);
        const BlockState* headState = movedState->setBoxedValue(
            static_cast<const Property*>(PistonHeadBlock::SHORT), &shortHeadValue);
        mBlockRenderer->tesselatePistonArmNoCulling(headState, position, false, headState);
    }
    else if (movingBlock->isSourcePiston() && !movingBlock->isExtending())
    {
        TypedBoxed<const Direction*> facing(movedState->getValue(PistonHeadBlock::FACING));
        const BlockState* pistonState = Blocks::piston->defaultBlockState()->setBoxedValue(
            static_cast<const Property*>(PistonBaseBlock::FACING), &facing);
        bool extended = true;
        TypedBoxed<bool> extendedValue(extended);
        pistonState = pistonState->setBoxedValue(
            static_cast<const Property*>(PistonBaseBlock::EXTENDED), &extendedValue);
        mBlockRenderer->tesselatePistonArmNoCulling(movedState, position, false, movedState);
        builder->offset((float)((float)x - (float)position.x),
                        (float)((float)y - (float)position.y),
                        (float)((float)z - (float)position.z));
        mBlockRenderer->tesselatePistonBaseForceExtended(pistonState, position, movedState);
    }
    else
    {
        mBlockRenderer->tesselateInWorldNoCulling(movedState, position, movedState, blockEntity);
    }

    builder->offset(0.0F, 0.0F, 0.0F);
    builder->color(1.0F, 1.0F, 1.0F);
    tesselator->end();
    Lighting::turnOn();
}

void PistonHeadRenderer::onNewLevel(Level* level)
{
    delete mBlockRenderer;
    mBlockRenderer = new BlockRenderer(level);
}
