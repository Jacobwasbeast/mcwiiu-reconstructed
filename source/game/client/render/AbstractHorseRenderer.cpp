#include "game/client/render/AbstractHorseRenderer.hpp"

#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/core/GlStateManager.hpp"

AbstractHorseRenderer::AbstractHorseRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius)
    : MobRenderer(dispatcher, model, shadowRadius)
{
    mScale = shadowRadius;
}

void AbstractHorseRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(mScale, mScale, mScale);
    LivingEntityRenderer::scale(entity, partialTicks);
}
