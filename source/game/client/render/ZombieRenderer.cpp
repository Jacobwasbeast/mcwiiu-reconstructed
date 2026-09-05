#include "game/client/render/ZombieRenderer.hpp"

#include <cmath>

#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ZombieModel.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

class Zombie;
class ZombieArmorLayer;

static ResourceLocation sZombieLocation;

ZombieRenderer::ZombieRenderer(EntityRenderDispatcher* dispatcher)
    : HumanoidMobRenderer(dispatcher, new ZombieModel(ModelDefinition()), 0.5F)
{
    addLayer(new ZombieArmorLayer(this, ENTITY_TYPE_ZOMBIE));
}

ZombieRenderer::~ZombieRenderer()
{
}

const ResourceLocation* ZombieRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &sZombieLocation;
}

void ZombieRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks,
                                    float rotationYaw, float partialTicks)
{
    boost::shared_ptr<Zombie> zombie = boost::static_pointer_cast<Zombie>(entity);
    if (zombie->isConverting())
    {
        rotationYaw += (float)(std::cos((double)zombie->mConversionTime * 3.25) *
                               3.1415927410125732 * 0.25);
    }

    LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);
}

Model* ZombieRenderer::createModel(const ModelDefinition& definition)
{
    return new HumanoidModel(definition, 0.0F, 0.0F, 64, 32, false, true, true);
}
