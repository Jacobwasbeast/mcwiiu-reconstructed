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
#include "game/world/block/Material.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/world/Level.hpp"
#include "game/core/Boxed.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/AbstractProperty.hpp"

static ResourceLocation sPistonTexture((_TEXTURE_NAME)0);

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
    if (movingBlock == NULL)
        return;

    BlockPos position = movingBlock->getBlockPos();
    const BlockState* movedState = movingBlock->getBlockState();
    const Block* movedBlock = movedState->getBlock();
    if (movedState->getMaterial() == Material::AIR)
        return;

    double progress = movingBlock->getProgress(partialTick);
    if (progress > 1.0)
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

    const double xOffset = movingBlock->getXOff(partialTick);
    const double yOffset = movingBlock->getYOff(partialTick);
    const double zOffset = movingBlock->getZOff(partialTick);
    builder->offset((float)((float)x - (float)position.x + xOffset),
                    (float)((float)y - (float)position.y + yOffset),
                    (float)((float)z - (float)position.z + zOffset));
    builder->color(1.0F, 1.0F, 1.0F);
    onNewLevel(getLevel());

    if (movedBlock == Blocks::piston_head && progress <= 0.25)
    {
        TypedBoxed<bool> shortHeadValue(true);
        const BlockState* headState = movedState->setBoxedValue(
            static_cast<const Property*>(PistonHeadBlock::SHORT), &shortHeadValue);
        mBlockRenderer->tesselatePistonArmNoCulling(headState, position, false, headState);
    }
    else if (movingBlock->isSourcePiston() && !movingBlock->isExtending())
    {
        TypedBoxed<PistonHeadBlock::PistonType*> pistonType(
            PistonHeadBlock::TYPE->getUnboxedValue(
                movedBlock == Blocks::sticky_piston ? L"sticky" : L"normal"));
        const BlockState* headState = Blocks::piston_head->defaultBlockState()->setBoxedValue(
            static_cast<const Property*>(PistonHeadBlock::TYPE), &pistonType);
        TypedBoxed<const Direction*> facing(movedState->getValue(PistonBaseBlock::FACING));
        headState = headState->setBoxedValue(
            static_cast<const Property*>(PistonHeadBlock::FACING), &facing);
        TypedBoxed<bool> shortHeadValue(progress >= 0.5);
        headState = headState->setBoxedValue(
            static_cast<const Property*>(PistonHeadBlock::SHORT), &shortHeadValue);
        mBlockRenderer->tesselatePistonArmNoCulling(headState, position, progress < 0.5, headState);

        builder->offset((float)((float)x - (float)position.x),
                        (float)((float)y - (float)position.y),
                        (float)((float)z - (float)position.z));
        TypedBoxed<bool> extendedValue(true);
        movedState->setBoxedValue(
            static_cast<const Property*>(PistonBaseBlock::EXTENDED), &extendedValue);
        mBlockRenderer->tesselatePistonBaseForceExtended(movedState, position, movedState);
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
