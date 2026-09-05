#include "game/client/render/GhastRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation GHAST_LOCATION;
static ResourceLocation GHAST_SHOOTING_LOCATION;

GhastRenderer::GhastRenderer(EntityRenderDispatcher*)
    : GhastModel(ModelDefinition())
{
}

GhastRenderer::~GhastRenderer()
{
}

void GhastRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    GlStateManager::scalef(4.5F, 4.5F, 4.5F);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
}

const ResourceLocation* GhastRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &GHAST_LOCATION;
}

Model* GhastRenderer::createModel(const ModelDefinition& definition)
{
    return new GhastModel(definition);
}
