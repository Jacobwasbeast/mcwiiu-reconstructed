#include "game/client/render/PigZombieRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/PigZombieModel.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/ZombieArmorLayer.hpp"

static ResourceLocation PIG_ZOMBIE_LOCATION;

PigZombieRenderer::PigZombieRenderer(EntityRenderDispatcher* dispatcher)
    : HumanoidMobRenderer(dispatcher, new PigZombieModel(ModelDefinition()), 0.5F)
{
    addLayer(new ZombieArmorLayer(this, ENTITY_TYPE_PIG_ZOMBIE));
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
