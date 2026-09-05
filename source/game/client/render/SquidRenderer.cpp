#include "game/client/render/SquidRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SquidModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"

struct SquidRenderState
{
    float mSquidPitch;
    float mOldSquidPitch;
    float mSquidYaw;
    float mOldSquidYaw;
    float mTentacleMovement;
    float mOldTentacleMovement;
};

SquidRenderer::SquidRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new SquidModel(ModelDefinition()), 0.7F)
{
}

SquidRenderer::~SquidRenderer()
{
}

void SquidRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float rotationYaw,
                                   float partialTicks, float ageInTicks)
{
    SquidRenderState* squid = (SquidRenderState*)entity.get();
    float bodyPitch = squid->mOldSquidPitch +
                      (squid->mSquidPitch - squid->mOldSquidPitch) * partialTicks;
    float bodyYaw = squid->mOldSquidYaw +
                    (squid->mSquidYaw - squid->mOldSquidYaw) * partialTicks;

    GlStateManager::translatef(0.0F, 0.5F, 0.0F);
    GlStateManager::rotatef(180.0F - rotationYaw, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(bodyPitch, 1.0F, 0.0F, 0.0F);
    GlStateManager::rotatef(bodyYaw, 0.0F, 1.0F, 0.0F);
    GlStateManager::translatef(0.0F, -1.2F, 0.0F);
}

double SquidRenderer::getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    SquidRenderState* squid = (SquidRenderState*)entity.get();
    return (double)(squid->mOldTentacleMovement +
                    (squid->mTentacleMovement - squid->mOldTentacleMovement) * partialTicks);
}

const ResourceLocation* SquidRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    static ResourceLocation location;
    return &location;
}

Model* SquidRenderer::createModel(const ModelDefinition& definition)
{
    return new SquidModel(definition);
}
