#include "game/client/render/LivingEntityRenderer.hpp"

#include <cmath>

#include "game/client/render/Model.hpp"
#include "game/client/render/RenderLayer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/GlStateManager.hpp"

double lerp(double from, double to, double amount)
{
    return (to - from) * amount + from;
}

LivingEntityRenderer::LivingEntityRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius)
    : EntityRenderer(dispatcher), mLayers(), mItemFrame(false)
{
    mModel = model;
    mShadowRadius = shadowRadius;
}

LivingEntityRenderer::~LivingEntityRenderer()
{
}

bool LivingEntityRenderer::addLayer(RenderLayer* layer)
{
    mLayers.push_back(layer);
    return true;
}

bool LivingEntityRenderer::removeLayer(RenderLayer* layer)
{
    std::vector<RenderLayer*>::iterator it = mLayers.begin();
    while (it != mLayers.end() && *it != layer)
        ++it;
    if (it == mLayers.end())
        return false;
    mLayers.erase(it);
    return true;
}

Model* LivingEntityRenderer::getModel()
{
    return mModel;
}

Model* LivingEntityRenderer::getModel(boost::shared_ptr<LivingEntity>&)
{
    return mModel;
}

float LivingEntityRenderer::rotlerp(float from, float to, float delta)
{
    float difference = to - from;
    while (difference < -180.0F)
        difference += 360.0F;
    while (difference >= 180.0F)
        difference -= 360.0F;
    return from + delta * difference;
}

void LivingEntityRenderer::translateWeaponItem()
{
}

float LivingEntityRenderer::setupScale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
    scale(entity, partialTicks);
    GlStateManager::translatef(0.0F, 3.0F, 0.0F);
    return 0.0625F;
}

bool LivingEntityRenderer::setupSolidState(boost::shared_ptr<LivingEntity>&)
{
    GlStateManager::disableLighting();
    GlStateManager::activeTexture(1);
    GlStateManager::disableTexture();
    GlStateManager::activeTexture(0);
    return true;
}

bool LivingEntityRenderer::setupOverlayColor(boost::shared_ptr<LivingEntity>&, float, bool overlay)
{
    if (!overlay)
        return false;
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    return true;
}

void LivingEntityRenderer::setupOverlayColor(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    setupOverlayColor(entity, partialTicks, true);
}

void LivingEntityRenderer::teardownOverlayColor()
{
    GlStateManager::color4f(0.0F, 0.0F, 0.0F, 0.0F);
}

void LivingEntityRenderer::tearDownSolidState()
{
    GlStateManager::enableLighting();
    GlStateManager::activeTexture(1);
    GlStateManager::enableTexture();
    GlStateManager::activeTexture(0);
}

void LivingEntityRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                  float partialTick)
{
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

void LivingEntityRenderer::setupForTexturePack(TexturePack* pack)
{
    EntityRenderer::setupForTexturePack(pack);
    std::vector<RenderLayer*>::iterator it = mLayers.begin();
    while (it != mLayers.end())
    {
        (*it)->setupForTexturePack(pack);
        ++it;
    }
}

void LivingEntityRenderer::renderModel(boost::shared_ptr<LivingEntity>&, float, float, float, float, float, float)
{
}

bool LivingEntityRenderer::isVisible(boost::shared_ptr<LivingEntity>&)
{
    return !mItemFrame;
}

void LivingEntityRenderer::setupPosition(boost::shared_ptr<LivingEntity>&, double x, double y, double z)
{
    GlStateManager::translatef((float)x, (float)y, (float)z);
}

void LivingEntityRenderer::setupRotations(boost::shared_ptr<LivingEntity>&, float, float rotationYaw, float)
{
    GlStateManager::rotatef(180.0F - rotationYaw, 0.0F, 1.0F, 0.0F);
}

float LivingEntityRenderer::getAttackAnim(boost::shared_ptr<LivingEntity>&, float)
{
    return mModel == NULL ? 0.0F : mModel->mAttackTime;
}

double LivingEntityRenderer::getBob(boost::shared_ptr<LivingEntity>&, float)
{
    return 0.0F;
}

void LivingEntityRenderer::renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                        float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                        float partialTick, bool overlay)
{
    std::vector<RenderLayer*>::iterator it = mLayers.begin();
    while (it != mLayers.end())
    {
        (*it)->render(entity, limbSwing, limbSwingAmount, partialTick, ageInTicks, netHeadYaw, headPitch, scale,
                      overlay);
        ++it;
    }
}

double LivingEntityRenderer::getFlipDegrees(boost::shared_ptr<LivingEntity>&)
{
    return 90.0F;
}

int LivingEntityRenderer::getOverlayColor(boost::shared_ptr<LivingEntity>&, float, float)
{
    return 0;
}

void LivingEntityRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
}

void LivingEntityRenderer::renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z)
{
    EntityRenderer::renderName(entity, x, y, z);
}

bool LivingEntityRenderer::shouldShowName(boost::shared_ptr<Entity>& entity)
{
    return EntityRenderer::shouldShowName(entity);
}
