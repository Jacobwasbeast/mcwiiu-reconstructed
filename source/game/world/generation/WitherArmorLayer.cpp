#include "game/world/generation/WitherArmorLayer.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/WitherBossModel.hpp"
#include "game/client/render/WitherBossRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation sWitherArmorLocation;

WitherArmorLayer::WitherArmorLayer(WitherBossRenderer* renderer)
    : mRenderer(renderer)
{
    ModelDefinition modelDefinition;
    mModel = new WitherBossModel(0.5f, modelDefinition);
    mOriginalModel = mModel;
}

WitherArmorLayer::~WitherArmorLayer()
{
}

void WitherArmorLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                              float limbSwingAmount, float partialTicks, float ageInTicks,
                              float netHeadYaw, float headPitch, float scale,
                              bool bUseDisplayList)
{
    boost::shared_ptr<WitherBoss> wither = boost::static_pointer_cast<WitherBoss>(entity);

    if (!wither->isPowered() || wither->IsSpawning())
    {
        return;
    }

    GlStateManager::depthMask(!wither->isInvisible());
    mRenderer->bindTexture(&sWitherArmorLocation);
    GlStateManager::matrixMode(5890);
    GlStateManager::loadIdentity();

    float textureOffset = (float)wither->mTickCount + partialTicks;
    GlStateManager::translatef(Mth::cos(textureOffset * 0.07f) * 3.0f,
                                textureOffset * 0.01f, 256.0f);

    GlStateManager::matrixMode(5888);
    GlStateManager::enableBlend();
    GlStateManager::color4f(0.5f, 0.5f, 0.5f, 1.0f);
    GlStateManager::disableLighting();
    GlStateManager::blendFunc(1, 1);

    boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
    mModel->copyPropertiesFrom(mRenderer->getModel());
    mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw,
                   headPitch, scale, bUseDisplayList);

    GlStateManager::matrixMode(5890);
    GlStateManager::loadIdentity();
    GlStateManager::matrixMode(5888);
    GlStateManager::enableLighting();
    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
}

bool WitherArmorLayer::colorsOnDamage()
{
    return false;
}

void WitherArmorLayer::setupForTexturePack(TexturePack* pTexturePack)
{
    if (mOriginalModel == NULL)
        mOriginalModel = mModel;

    if (mModel != mOriginalModel)
    {
        if (mModel != NULL)
            delete mModel;
        mModel = NULL;
    }

    std::wstring modelName = L"witherBoss";
    if (!pTexturePack->HasCustomModel(modelName))
    {
        mModel = mOriginalModel;
    }
    else
    {
        mModel = new WitherBossModel(0.5f, *pTexturePack->GetCustomModel(modelName));
    }
}
