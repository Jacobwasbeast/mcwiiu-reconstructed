#include "game/client/render/ChestRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/ChestBlockEntity.hpp"

static ResourceLocation LARGE_TRAPPED_CHEST_LOCATION(TN_ITEM_TRAPPED_DOUBLE);
static ResourceLocation LARGE_CHEST_LOCATION(TN_ITEM_LARGECHEST);
static ResourceLocation TRAPPED_CHEST_LOCATION(TN_ITEM_TRAPPED);
static ResourceLocation CHEST_LOCATION(TN_ITEM_CHEST);

ChestRenderer::ChestRenderer() : BlockEntityRenderer()
{
    mChestModel = new ChestModel();
    mLargeChestModel = new LargeChestModel();
}

ChestRenderer::~ChestRenderer()
{
    delete mChestModel;
    delete mLargeChestModel;
}

void ChestRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                           int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    boost::shared_ptr<ChestBlockEntity> chest = boost::static_pointer_cast<ChestBlockEntity>(blockEntity);

    GlStateManager::enableDepthTest();
    GlStateManager::depthFunc(3);
    GlStateManager::depthMask(true);

    int data;
    if (chest->hasLevel())
    {
        data = chest->getData();
        chest->checkNeighbors();
    }
    else
    {
        data = 0;
    }

    if (chest->mNorthNeighbour.lock() || chest->mWestNeighbour.lock())
    {
        return;
    }

    ChestModel* model;
    if (chest->mEastNeighbour.lock() || chest->mSouthNeighbour.lock())
    {
        model = mLargeChestModel;
        if (breakProgress >= 0)
        {
            bindTexture(&BREAKING_LOCATIONS[breakProgress]);

            GlStateManager::matrixMode(2);
            GlStateManager::pushMatrix();
            GlStateManager::scalef(8.0F, 4.0F, 1.0F);
            GlStateManager::translatef(0.0625F, 0.0625F, 0.0625F);
            GlStateManager::matrixMode(0);
            GlStateManager::pushMatrix();
            GlStateManager::enableRescaleNormal();
        }
        else
        {
            if (chest->getType() == 1)
            {
                bindTexture(&LARGE_TRAPPED_CHEST_LOCATION);
            }
            else
            {
                bindTexture(&LARGE_CHEST_LOCATION);
            }

            GlStateManager::pushMatrix();
            GlStateManager::enableRescaleNormal();
        }
    }
    else
    {
        model = mChestModel;
        if (breakProgress >= 0)
        {
            bindTexture(&BREAKING_LOCATIONS[breakProgress]);

            GlStateManager::matrixMode(2);
            GlStateManager::pushMatrix();
            GlStateManager::scalef(4.0F, 4.0F, 1.0F);
            GlStateManager::translatef(0.0625F, 0.0625F, 0.0625F);
            GlStateManager::matrixMode(0);
            GlStateManager::pushMatrix();
            GlStateManager::enableRescaleNormal();
        }
        else
        {
            if (chest->getType() == 1)
            {
                bindTexture(&TRAPPED_CHEST_LOCATION);
            }
            else
            {
                bindTexture(&CHEST_LOCATION);
            }

            GlStateManager::pushMatrix();
            GlStateManager::enableRescaleNormal();
        }
    }

    if (bTranslucent)
    {
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
    }

    GlStateManager::translatef((float)x, (float)y + 1.0F, (float)z + 1.0F);
    GlStateManager::scalef(1.0F, -1.0F, -1.0F);
    GlStateManager::translatef(0.5F, 0.5F, 0.5F);

    float yRot = 0.0F;
    if (data == 2)
    {
        yRot = 180.0F;
    }
    if (data == 3)
    {
        yRot = 0.0F;
    }
    if (data == 4)
    {
        yRot = 90.0F;
    }
    if (data == 5)
    {
        yRot = -90.0F;
    }

    if (data == 2 && chest->mEastNeighbour.lock())
    {
        GlStateManager::translatef(1.0F, 0.0F, 0.0F);
    }

    if (data == 5 && chest->mSouthNeighbour.lock())
    {
        GlStateManager::translatef(0.0F, 0.0F, -1.0F);
    }

    GlStateManager::rotatef(yRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::translatef(-0.5F, -0.5F, -0.5F);

    float openness = chest->mOpenOld + (chest->mOpen - chest->mOpenOld) * a;
    if (chest->mNorthNeighbour.lock())
    {
        float neighbourOpenness = chest->mNorthNeighbour.lock()->mOpenOld
                                  + (chest->mNorthNeighbour.lock()->mOpen - chest->mNorthNeighbour.lock()->mOpenOld)
                                        * a;
        if (neighbourOpenness > openness)
        {
            openness = neighbourOpenness;
        }
    }

    if (chest->mWestNeighbour.lock())
    {
        float neighbourOpenness = chest->mWestNeighbour.lock()->mOpenOld
                                  + (chest->mWestNeighbour.lock()->mOpen - chest->mWestNeighbour.lock()->mOpenOld) * a;
        if (neighbourOpenness > openness)
        {
            openness = neighbourOpenness;
        }
    }

    openness = 1.0F - openness;
    openness = 1.0F - openness * openness * openness;

    model->mLid->mXRot = -(openness * 1.5707964F);
    model->render(bUseDisplayList);

    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();

    if (bTranslucent)
    {
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    }

    if (breakProgress >= 0)
    {
        GlStateManager::matrixMode(2);
        GlStateManager::popMatrix();
        GlStateManager::matrixMode(0);
    }
}
