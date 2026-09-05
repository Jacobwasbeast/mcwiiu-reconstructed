#include "game/client/render/PigZombieRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation PIG_ZOMBIE_LOCATION;

PigZombieRenderer::PigZombieRenderer(EntityRenderDispatcher*)
    : PigZombieModel(ModelDefinition())
{
}

PigZombieRenderer::~PigZombieRenderer()
{
}

const ResourceLocation* PigZombieRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &PIG_ZOMBIE_LOCATION;
}

std::wstring PigZombieRenderer::getModelName()
{
    return L"pigzombie";
}

Model* PigZombieRenderer::createModel(const ModelDefinition& definition)
{
    return new PigZombieModel(definition);
}
