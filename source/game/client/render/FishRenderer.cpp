#include "game/client/render/FishRenderer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/FishModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation FISH_TEXTURE(TN_MOB_FISH_COD);
static ResourceLocation PUFFERFISH_TEXTURE(TN_MOB_FISH_PUFFERFISH);
static ResourceLocation SALMON_TEXTURE(TN_MOB_FISH_SALMON);

FishRenderer::FishRenderer(EntityRenderDispatcher* dispatcher, FishType fishType, float shadowRadius)
    : MobRenderer(dispatcher, new FishModel(ModelDefinition(), fishType, 0), shadowRadius), mFishType(fishType)
{
}

FishRenderer::~FishRenderer()
{
    return;
}

Model* FishRenderer::getModel(boost::shared_ptr<LivingEntity>& entity)
{
    if (entity->GetType() == EntityType::PUFFERFISH)
        return LivingEntityRenderer::getModel(entity);
    return mModel;
}

const ResourceLocation* FishRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    if (entity->GetType() != EntityType::FISH)
    {
        if (entity->GetType() == EntityType::PUFFERFISH)
            return &PUFFERFISH_TEXTURE;
        if (entity->GetType() == EntityType::SALMON)
            return &SALMON_TEXTURE;
    }
    return &FISH_TEXTURE;
}

Model* FishRenderer::createModel(const ModelDefinition& definition)
{
    return new FishModel(definition, mFishType, 0);
}
