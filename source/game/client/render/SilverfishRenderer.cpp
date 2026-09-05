#include "game/client/render/SilverfishRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/util/ResourceLocation.hpp"

SilverfishRenderer::SilverfishRenderer(EntityRenderDispatcher*)
    : SilverfishModel(ModelDefinition())
{
}

SilverfishRenderer::~SilverfishRenderer()
{
}

double SilverfishRenderer::getFlipDegrees(boost::shared_ptr<LivingEntity>&)
{
    return 180.0;
}

const ResourceLocation* SilverfishRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    static ResourceLocation location;
    return &location;
}

Model* SilverfishRenderer::createModel(const ModelDefinition& definition)
{
    return new SilverfishModel(definition);
}
