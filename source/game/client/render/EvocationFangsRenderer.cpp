#include "game/client/render/EvocationFangsRenderer.hpp"

#include "game/client/render/EvocationFangsModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/EvocationFangs.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

EvocationFangsRenderer::EvocationFangsRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
    mFangsModel = new EvocationFangsModel(ModelDefinition());
}

EvocationFangsRenderer::~EvocationFangsRenderer()
{
    delete mFangsModel;
}

void EvocationFangsRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                    float yaw, float partialTick)
{
    boost::shared_ptr<EvocationFangs> fangs = boost::static_pointer_cast<EvocationFangs>(entity);
    double progress = fangs->getAnimationProgress(partialTick);
    float scale = 0.5F;

    if (progress == 1.0)
    {
        return;
    }

    if (progress > 0.9f)
    {
        scale = static_cast<float>((1.0 - progress) / 0.1f);
        scale += scale;
    }

    GlStateManager::pushMatrix();
    GlStateManager::disableCull();
    GlStateManager::enableAlphaTest();
    bindTexture(entity);
    GlStateManager::translatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    GlStateManager::rotatef(90.0F - fangs->mYRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::scalef(-scale, -scale, scale);
    GlStateManager::translatef(0.0F, -0.625F, 0.0F);
    mFangsModel->render(entity, static_cast<float>(progress), 0.5F, 1.75F, fangs->mYRot,
                   fangs->mXRot, 0.03125F, true);
    GlStateManager::popMatrix();
    GlStateManager::enableCull();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* EvocationFangsRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    static ResourceLocation location((_TEXTURE_NAME)197);
    return &location;
}

Model* EvocationFangsRenderer::createModel(const ModelDefinition& definition)
{
    return new EvocationFangsModel(definition);
}
