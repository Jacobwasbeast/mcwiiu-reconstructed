#include "game/client/render/SnowManRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Model.hpp"
#include "game/util/ResourceLocation.hpp"

SnowManRenderer::SnowManRenderer(EntityRenderDispatcher*)
    : SnowManModel(ModelDefinition())
{
}

SnowManRenderer::~SnowManRenderer()
{
}

const ResourceLocation* SnowManRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    static ResourceLocation location;
    return &location;
}

Model* SnowManRenderer::getModel()
{
    return this;
}

Model* SnowManRenderer::createModel(const ModelDefinition& definition)
{
    return new SnowManModel(definition);
}
