#include "game/client/render/MushroomCowRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation MUSHROOM_COW_LOCATION;

MushroomCowRenderer::MushroomCowRenderer(EntityRenderDispatcher* dispatcher)
    : CowModel(ModelDefinition())
{
}

MushroomCowRenderer::~MushroomCowRenderer()
{
}

const ResourceLocation* MushroomCowRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &MUSHROOM_COW_LOCATION;
}

Model* MushroomCowRenderer::createModel(const ModelDefinition& definition)
{
    return new CowModel(definition);
}
