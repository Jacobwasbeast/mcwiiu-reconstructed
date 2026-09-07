#include "game/world/generation/EnderEyesLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/EndermanRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation ENDER_EYES_LOCATION;

EnderEyesLayer::EnderEyesLayer(EndermanRenderer* renderer)
    : mRenderer(renderer)
{
}

EnderEyesLayer::~EnderEyesLayer()
{
}

void EnderEyesLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                            float partialTicks, float ageInTicks, float netHeadYaw, float headPitch,
                            float scale, bool bUseDisplayList)
{
    mRenderer->bindTexture(&ENDER_EYES_LOCATION);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(1, 1);
    GlStateManager::disableLighting();
    GlStateManager::depthMask(!entity->isInvisible());
    GlStateManager::multiTexCoord2f(61680.0f, 0.5f);
    GlStateManager::enableLighting();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
    mRenderer->getModel()->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw,
                                   headPitch, scale, bUseDisplayList);
    mRenderer->setLightColor(renderEntity);

    GlStateManager::depthMask(true);
    GlStateManager::disableBlend();
    GlStateManager::enableAlphaTest();
}

bool EnderEyesLayer::colorsOnDamage()
{
    return false;
}
