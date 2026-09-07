#include "game/client/render/ShulkerBoxRenderer.hpp"

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"

static ResourceLocation SHULKER_BOX_LOCATION;

ShulkerBoxRenderer::ShulkerBoxRenderer(ShulkerModel* model)
    : mModel(model), mShulkerModel(model)
{
}

ShulkerBoxRenderer::~ShulkerBoxRenderer()
{
}

void ShulkerBoxRenderer::setupForTexturePack(TexturePack* texturePack)
{
    if (mShulkerModel == NULL)
    {
        mShulkerModel = static_cast<ShulkerModel*>(mModel);
    }

    if (mModel != mShulkerModel)
    {
        if (mModel != NULL)
        {
            mModel->clearAllOffsets();
        }
        mModel = NULL;
    }

    std::wstring modelName = mShulkerModel->getName();
    ModelDefinition* definition = texturePack->GetCustomModel(modelName);
    if (definition == NULL)
    {
        mModel = mShulkerModel;
    }
    else
    {
        mModel = new ShulkerModel(*definition);
    }
}

void ShulkerBoxRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                                float partialTick, int breakProgress, bool translucent, float alpha,
                                bool useDisplayList)
{
    boost::shared_ptr<ShulkerBoxBlockEntity> shulker =
        boost::static_pointer_cast<ShulkerBoxBlockEntity>(blockEntity);

    const Direction* direction = Direction::DOWN;
    if (shulker->hasLevel())
    {
        direction = shulker->getLevel()->getBlockState(shulker->getBlockPos())
            ->getValue(DirectionalBlock::FACING_PROPERTY);
    }

    GlStateManager::enableDepthTest();
    GlStateManager::depthFunc(3);
    GlStateManager::depthMask(true);
    GlStateManager::disableCull();

    if (breakProgress < 0)
    {
        bindTexture(&SHULKER_BOX_LOCATION);
    }
    else
    {
        bindTexture(&BREAKING_LOCATIONS[breakProgress]);
        GlStateManager::matrixMode(2);
        GlStateManager::pushMatrix();
        GlStateManager::scalef(4.0F, 4.0F, 1.0F);
        GlStateManager::translatef(0.0625F, 0.0625F, 0.0625F);
        GlStateManager::matrixMode(0);
    }

    GlStateManager::pushMatrix();
    GlStateManager::enableRescaleNormal();
    GlStateManager::translatef((float)x + 0.5F, (float)y + 1.5F, (float)z + 0.5F);
    GlStateManager::scalef(1.0F, -1.0F, -1.0F);
    GlStateManager::translatef(0.0F, -1.0F, 0.0F);
    GlStateManager::scalef(0.5F, 0.5F, 0.5F);

    const int facing = direction->get3DDataValue();
    if (facing == 0)
    {
        GlStateManager::translatef(0.0F, 2.0F, 0.0F);
        GlStateManager::rotatef(180.0F, 1.0F, 0.0F, 0.0F);
    }
    else if (facing == 2)
    {
        GlStateManager::translatef(0.0F, 1.0F, 1.0F);
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager::rotatef(180.0F, 0.0F, 0.0F, 1.0F);
    }
    else if (facing == 3)
    {
        GlStateManager::translatef(0.0F, 1.0F, 0.0F);
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
    }
    else if (facing == 4)
    {
        GlStateManager::translatef(1.0F, 1.0F, 0.0F);
        GlStateManager::rotatef(90.0F, 0.0F, 0.0F, 1.0F);
    }
    else if (facing == 5)
    {
        GlStateManager::translatef(0.0F, 1.0F, 0.0F);
        GlStateManager::rotatef(-90.0F, 0.0F, 0.0F, 1.0F);
    }

    if (translucent)
    {
        const unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(
            shulker->getColor()->getShulkerColor());
        GlStateManager::color4f((float)((colour >> 16) & 0xFF) / 255.0F,
                                (float)((colour >> 8) & 0xFF) / 255.0F,
                                (float)(colour & 0xFF) / 255.0F, alpha);
    }

    ShulkerModel* model = static_cast<ShulkerModel*>(mModel);
    const float progress = shulker->getProgress(partialTick);
    model->mLid->mY = 24.0F - 16.0F * (0.5F + 0.5F * progress);
    model->mLid->mXRot = progress * 4.712389F;

    if (mModel->hasOverriddenParts())
    {
        model->mBase->render(0.0625F, useDisplayList, false);
        model->mLid->render(0.0625F, useDisplayList, false);
        model->mHead->render(0.0625F, useDisplayList, false);
        mModel->callSkinnedDrawlist();
    }
    else
    {
        model->mBase->render(0.0625F, useDisplayList, false);
        model->mLid->render(0.0625F, useDisplayList, false);
        model->mHead->render(0.0625F, useDisplayList, false);
    }

    GlStateManager::enableCull();
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

    if (breakProgress >= 0)
    {
        GlStateManager::matrixMode(2);
        GlStateManager::popMatrix();
        GlStateManager::matrixMode(0);
    }
}
