#include "game/client/render/VindicationIllagerRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/IllagerModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation vindicationIllagerLocation;

VindicationIllagerRenderer::VindicationIllagerRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new IllagerModel(0.0f, 0.0f, 64, 64, ModelDefinition()), 0.5f)
{
    addLayer(new ItemInHandLayer(this));
}

VindicationIllagerRenderer::~VindicationIllagerRenderer()
{
}

void VindicationIllagerRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                        float yaw, float partialTick)
{
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* VindicationIllagerRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &vindicationIllagerLocation;
}

void VindicationIllagerRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(0.9375f, 0.9375f, 0.9375f);
}

Model* VindicationIllagerRenderer::createModel(const ModelDefinition& definition)
{
    return new IllagerModel(0.0f, 0.0f, 64, 64, definition);
}
