#include "game/client/render/MushroomCowRenderer.hpp"

#include "game/client/render/CowModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/MushroomCowMushroomLayer.hpp"

static ResourceLocation MUSHROOM_COW_LOCATION;

MushroomCowRenderer::MushroomCowRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new CowModel(ModelDefinition()), 0.7f)
{
    addLayer(new MushroomCowMushroomLayer(this));
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
