#include "game/client/render/ChestRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/ChestBlockEntity.hpp"

static ResourceLocation CHEST_LOCATION((_TEXTURE_NAME)54);
static ResourceLocation LARGE_CHEST_LOCATION((_TEXTURE_NAME)55);
static ResourceLocation TRAPPED_CHEST_LOCATION((_TEXTURE_NAME)57);
static ResourceLocation LARGE_TRAPPED_CHEST_LOCATION((_TEXTURE_NAME)58);

ChestRenderer::ChestRenderer()
    : mChestModel(NULL), mLargeChestModel(NULL)
{
    mChestModel = new ChestModel(ModelDefinition());
    mLargeChestModel = new LargeChestModel(ModelDefinition());
}

ChestRenderer::~ChestRenderer()
{
    delete mChestModel;
    delete mLargeChestModel;
}

void ChestRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                           float a, int breakProgress, bool bTranslucent, float alpha,
                           bool bUseDisplayList)
{
    boost::shared_ptr<ChestBlockEntity> chest = boost::static_pointer_cast<ChestBlockEntity>(blockEntity);
    int data = 0;
    boost::shared_ptr<ChestBlockEntity> neighbour;
    bool large = false;
    bool trapped = false;

    if (chest->hasLevel())
    {
        data = chest->getData();
        neighbour = chest->getNeighbour();
        large = neighbour.get() != NULL;
        trapped = chest->getType() != 0;
    }

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
        if (large)
        {
            if (trapped)
            {
                bindTexture(&LARGE_TRAPPED_CHEST_LOCATION);
            }
            else
            {
                bindTexture(&LARGE_CHEST_LOCATION);
            }
        }
        else
        {
            if (trapped)
            {
                bindTexture(&TRAPPED_CHEST_LOCATION);
            }
            else
            {
                bindTexture(&CHEST_LOCATION);
            }
        }

        GlStateManager::pushMatrix();
        GlStateManager::enableRescaleNormal();
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
    else if (data == 4)
    {
        yRot = 90.0F;
    }
    else if (data == 5)
    {
        yRot = -90.0F;
    }

    GlStateManager::rotatef(yRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::translatef(-0.5F, -0.5F, -0.5F);

    float openness = chest->mOOpenness + (chest->mOpenness - chest->mOOpenness) * a;
    if (large)
    {
        float neighbourOpenness = neighbour->mOOpenness +
            (neighbour->mOpenness - neighbour->mOOpenness) * a;
        if (neighbourOpenness > openness)
        {
            openness = neighbourOpenness;
        }
    }
    openness = 1.0F - openness;
    openness = 1.0F - openness * openness * openness;

    ChestModel* model = mChestModel;
    if (large)
    {
        model = mLargeChestModel;
    }
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
