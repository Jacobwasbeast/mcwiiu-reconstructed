#include "game/client/render/VillagerGolemRenderer.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/VillagerGolemModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/world/generation/VillagerGolemFlowerLayer.hpp"

static ResourceLocation sTextureLocation;

VillagerGolemRenderer::VillagerGolemRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new VillagerGolemModel(0.0F, -7.0F, ModelDefinition()), 0.5F)
{
    addLayer(new VillagerGolemFlowerLayer(this));
}

VillagerGolemRenderer::~VillagerGolemRenderer()
{
}

const ResourceLocation* VillagerGolemRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &sTextureLocation;
}

Model* VillagerGolemRenderer::createModel(const ModelDefinition& definition)
{
    return new VillagerGolemModel(0.0F, -7.0F, definition);
}

void VillagerGolemRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity,
                                           float ageInTicks, float rotationYaw,
                                           float partialTicks)
{
    boost::shared_ptr<VillagerGolem> golem = boost::static_pointer_cast<VillagerGolem>(entity);
    LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);

    if (golem->mWalkDistO >= 0.01F)
    {
        float walkCycle = fmodf(golem->mWalkDist - golem->mWalkDistO * (1.0F - partialTicks) + 6.0F,
                                13.0F);
        float rotation = (fabsf(walkCycle - 6.5F) - 3.25F) / 3.25F;
        GlStateManager::rotatef(rotation * 6.5F, 0.0F, 0.0F, 1.0F);
    }
}
