#include "game/client/render/SnowManRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/SnowManModel.hpp"
#include "game/world/generation/SnowManHeadLayer.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation SNOWMAN_LOCATION;

SnowManRenderer::SnowManRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new SnowManModel(ModelDefinition()), 0.5f)
{
    addLayer(new SnowManHeadLayer(this));
}

SnowManRenderer::~SnowManRenderer()
{
}

const ResourceLocation* SnowManRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &SNOWMAN_LOCATION;
}

Model* SnowManRenderer::getModel()
{
    return LivingEntityRenderer::getModel();
}

Model* SnowManRenderer::createModel(const ModelDefinition& definition)
{
    return new SnowManModel(definition);
}
