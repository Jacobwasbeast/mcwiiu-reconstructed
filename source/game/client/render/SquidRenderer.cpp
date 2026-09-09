#include "game/client/render/SquidRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SquidModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Squid.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation SQUID_LOCATION((_TEXTURE_NAME)25);

SquidRenderer::SquidRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new SquidModel(ModelDefinition()), 0.7F)
{
}

SquidRenderer::~SquidRenderer()
{
}

void SquidRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float, float rotationYaw,
                                   float partialTicks)
{
    boost::shared_ptr<Squid> squid = boost::static_pointer_cast<Squid>(entity);
    float bodyXRot = (squid->mXBodyRot - squid->mXBodyRotO) * partialTicks + squid->mXBodyRotO;
    float bodyZRot = (squid->mZBodyRot - squid->mZBodyRotO) * partialTicks + squid->mZBodyRotO;

    GlStateManager::translatef(0.0F, 0.5F, 0.0F);
    GlStateManager::rotatef(180.0F - rotationYaw, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(bodyXRot, 1.0F, 0.0F, 0.0F);
    GlStateManager::rotatef(bodyZRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::translatef(0.0F, -1.2F, 0.0F);
}

double SquidRenderer::getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<Squid> squid = boost::static_pointer_cast<Squid>(entity);
    return (squid->mTentacleAngle - squid->mOldTentacleAngle) * partialTicks + squid->mOldTentacleAngle;
}

const ResourceLocation* SquidRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &SQUID_LOCATION;
}

Model* SquidRenderer::createModel(const ModelDefinition& definition)
{
    return new SquidModel(definition);
}
