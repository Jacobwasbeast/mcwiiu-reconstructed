#include "game/client/render/SkullBlockRenderer.hpp"

#include <cmath>

#include "game/client/render/CreeperHeadModel.hpp"
#include "game/client/render/DragonHeadModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SkeletonHeadModel.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/SkullBlockEntity.hpp"

static ResourceLocation SKULL_TEXTURE_0((_TEXTURE_NAME)34);
static ResourceLocation SKULL_TEXTURE_1((_TEXTURE_NAME)82);
static ResourceLocation SKULL_TEXTURE_2((_TEXTURE_NAME)32);
static ResourceLocation SKULL_TEXTURE_3((_TEXTURE_NAME)37);
static ResourceLocation SKULL_TEXTURE_4((_TEXTURE_NAME)29);
static ResourceLocation SKULL_TEXTURE_5((_TEXTURE_NAME)59);

SkullBlockRenderer* SkullBlockRenderer::instance = NULL;

SkullBlockRenderer::SkullBlockRenderer()
    : mCreeperModel(NULL), mDragonModel(NULL), mActiveCreeperModel(NULL), mActiveDragonModel(NULL)
{
    mSkeletonModels[0] = new SkeletonHeadModel(0, 0, 64, 32, ModelDefinition());
    mSkeletonModels[1] = new SkeletonHeadModel(0, 0, 64, 32, ModelDefinition());
    mSkeletonModels[2] = new SkeletonHeadModel(0, 0, 64, 32, ModelDefinition());
    mSkeletonModels[3] = new SkeletonHeadModel(0, 0, 64, 64, ModelDefinition());
    mCreeperModel = new CreeperHeadModel(0.0F, ModelDefinition());
    mDragonModel = new DragonHeadModel(0.0F, ModelDefinition());
    mActiveCreeperModel = mCreeperModel;
    mActiveDragonModel = mDragonModel;
    mActiveSkeletonModels[0] = mSkeletonModels[0];
    mActiveSkeletonModels[1] = mSkeletonModels[1];
    mActiveSkeletonModels[2] = mSkeletonModels[2];
    mActiveSkeletonModels[3] = mSkeletonModels[3];
}

SkullBlockRenderer::~SkullBlockRenderer()
{
    if (mActiveSkeletonModels[3] != mSkeletonModels[3] && mActiveSkeletonModels[3] != NULL)
        delete mActiveSkeletonModels[3];
    if (mActiveCreeperModel != mCreeperModel && mActiveCreeperModel != NULL)
        delete mActiveCreeperModel;
    if (mActiveSkeletonModels[0] != mSkeletonModels[0] && mActiveSkeletonModels[0] != NULL)
        delete mActiveSkeletonModels[0];
    if (mActiveSkeletonModels[1] != mSkeletonModels[1] && mActiveSkeletonModels[1] != NULL)
        delete mActiveSkeletonModels[1];
    if (mActiveSkeletonModels[2] != mSkeletonModels[2] && mActiveSkeletonModels[2] != NULL)
        delete mActiveSkeletonModels[2];
    if (mActiveDragonModel != mDragonModel && mActiveDragonModel != NULL)
        delete mActiveDragonModel;
    delete mSkeletonModels[0];
    delete mSkeletonModels[1];
    delete mSkeletonModels[2];
    delete mSkeletonModels[3];
    delete mCreeperModel;
    delete mDragonModel;
}

void SkullBlockRenderer::renderSkull(float x, float y, float z, const Direction* direction,
                                     float rotation, float mouthAnimation, int skullType,
                                     int destroyStage, float scale, bool useDisplayList)
{
    Model* model = NULL;
    if (skullType == 4)
        model = mActiveCreeperModel;
    else if (skullType == 5)
        model = mActiveDragonModel;
    else if (skullType >= 0 && skullType < 4)
        model = mActiveSkeletonModels[skullType];
    else
        model = mActiveSkeletonModels[0];

    if (destroyStage >= 0)
    {
        bindTexture(&BREAKING_LOCATIONS[destroyStage]);
        GlStateManager::matrixMode(2);
        GlStateManager::pushMatrix();
        GlStateManager::scalef(2.0F, -2.0F, 1.0F);
        GlStateManager::translatef(0.0F, 0.0F, 0.0F);
        GlStateManager::matrixMode(0);
        GlStateManager::pushMatrix();
        GlStateManager::disableCull();
    }
    else
    {
        if (skullType == 1)
            bindTexture(&SKULL_TEXTURE_1);
        else if (skullType == 2)
            bindTexture(&SKULL_TEXTURE_2);
        else if (skullType == 3)
            bindTexture(&SKULL_TEXTURE_3);
        else if (skullType == 4)
            bindTexture(&SKULL_TEXTURE_4);
        else if (skullType == 5)
            bindTexture(&SKULL_TEXTURE_5);
        else
            bindTexture(&SKULL_TEXTURE_0);
        GlStateManager::pushMatrix();
        GlStateManager::disableCull();
    }

    if (direction == NULL)
    {
        GlStateManager::translatef(x + 0.5F, y, z + 0.5F);
        GlStateManager::enableRescaleNormal();
        GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
        GlStateManager::enableAlphaTest();
    }
    else
    {
        const float offset = 0.25F;
        unsigned int data = direction->get3DDataValue();
        if (data < 2)
            GlStateManager::translatef(x + 0.26F, y + offset, z + 0.26F);
        else if (data == 2)
        {
            GlStateManager::translatef(x + 0.5F, y + offset, z + 0.74F);
            GlStateManager::enableRescaleNormal();
            GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
            GlStateManager::enableAlphaTest();
        }
        else if (data < 4)
        {
            GlStateManager::translatef(x + 0.5F, y + offset, z + 0.26F);
            rotation = 180.0F;
            GlStateManager::enableRescaleNormal();
            GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
            GlStateManager::enableAlphaTest();
        }
        else if (data == 4)
        {
            GlStateManager::translatef(x + 0.74F, y + offset, z + 0.5F);
            rotation = -90.0F;
            GlStateManager::enableRescaleNormal();
            GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
            GlStateManager::enableAlphaTest();
        }
        else
        {
            GlStateManager::translatef(x + 0.26F, y + offset, z + 0.5F);
            rotation = 90.0F;
            GlStateManager::enableRescaleNormal();
            GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
            GlStateManager::enableAlphaTest();
        }
    }

    boost::shared_ptr<Entity> entity;
    model->render(entity, mouthAnimation, 0.0F, 0.0F, rotation, scale, 0.0625F, useDisplayList);
    GlStateManager::popMatrix();
    if (destroyStage >= 0)
    {
        GlStateManager::matrixMode(2);
        GlStateManager::popMatrix();
        GlStateManager::matrixMode(0);
    }
}

void SkullBlockRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y,
                                double z, float partialTicks, int breakProgress,
                                bool bTranslucent, float alpha, bool bUseDisplayList)
{
    boost::shared_ptr<SkullBlockEntity> skull = boost::static_pointer_cast<SkullBlockEntity>(blockEntity);
    const Direction* direction = Direction::from3DDataValue(skull->getData() & 7);
    if (skull->isRenderReady())
    {
        float mouthAnimation = (float)skull->getMouthAnimation(partialTicks);
        float rotation = (float)(skull->getRotation() * 360 - 180) * 0.0625F;
        renderSkull((float)x, (float)y, (float)z, direction, rotation, mouthAnimation,
                    skull->getSkullType(), breakProgress, alpha, bUseDisplayList);
    }
}

void SkullBlockRenderer::init(BlockEntityRenderDispatcher* pDispatcher)
{
    BlockEntityRenderer::init(pDispatcher);
    instance = this;
}

bool SkullBlockRenderer::dragonJawCheck()
{
    return mDragonModel->hasJawPart();
}

void SkullBlockRenderer::updateModel(int modelIndex, const std::wstring& modelName,
                                     const std::wstring& originalModelName, TexturePack* pTexturePack)
{
    SkeletonHeadModel*& model = mActiveSkeletonModels[modelIndex];
    SkeletonHeadModel* original = mSkeletonModels[modelIndex];
    if (model == NULL)
        model = original;
    if (model != original)
    {
        delete model;
        model = NULL;
    }
    if (pTexturePack->HasCustomModel(modelName))
        model = new SkeletonHeadModel(0, 0, 64, 32, *pTexturePack->GetCustomModel(modelName));
    else if (pTexturePack->HasCustomModel(originalModelName))
        model = new SkeletonHeadModel(0, 0, 64, 32, *pTexturePack->GetCustomModel(originalModelName));
    else
        model = original;
}

void SkullBlockRenderer::setupForTexturePack(TexturePack* pTexturePack)
{
    updateModel(0, L"skeleton_head", L"skeleton", pTexturePack);
    updateModel(1, L"skeleton_wither_head", L"skeleton.wither", pTexturePack);
    updateModel(2, L"zombie_head", L"zombie", pTexturePack);

    if (mActiveDragonModel == NULL)
        mActiveDragonModel = mDragonModel;
    if (pTexturePack->HasCustomModel(L"dragon_head"))
    {
        if (mActiveDragonModel != mDragonModel)
            delete mActiveDragonModel;
        mActiveDragonModel = new DragonHeadModel(0.0F, *pTexturePack->GetCustomModel(L"dragon_head"));
    }
    else
    {
        if (mActiveDragonModel != mDragonModel)
            delete mActiveDragonModel;
        mActiveDragonModel = mDragonModel;
    }

    if (mActiveCreeperModel == NULL)
        mActiveCreeperModel = mCreeperModel;
    if (pTexturePack->HasCustomModel(L"creeper_head"))
    {
        if (mActiveCreeperModel != mCreeperModel)
            delete mActiveCreeperModel;
        mActiveCreeperModel = new CreeperHeadModel(0.0F, *pTexturePack->GetCustomModel(L"creeper_head"));
    }
    else
    {
        if (mActiveCreeperModel != mCreeperModel)
            delete mActiveCreeperModel;
        mActiveCreeperModel = mCreeperModel;
    }
}
