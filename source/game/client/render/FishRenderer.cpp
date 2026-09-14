#include "game/client/render/FishRenderer.hpp"

#include "game/client/render/FishModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation FISH_TEXTURE;
static ResourceLocation PUFFERFISH_TEXTURE;
static ResourceLocation SALMON_TEXTURE;

FishRenderer::FishRenderer(EntityRenderDispatcher* dispatcher, FishType fishType, float shadowRadius)
    : MobRenderer(dispatcher, new FishModel(ModelDefinition(), fishType, 0), shadowRadius), mFishType(fishType)
{
}

FishRenderer::~FishRenderer()
{
}

Model* FishRenderer::getModel(boost::shared_ptr<LivingEntity>& entity)
{
    if (entity->GetType() == 0x000000000D502003ULL)
        return LivingEntityRenderer::getModel(entity);
    return mModel;
}

const ResourceLocation* FishRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    if (entity->GetType() != 0x000000000D502002ULL)
    {
        if (entity->GetType() == 0x000000000D502003ULL)
            return &PUFFERFISH_TEXTURE;
        if (entity->GetType() == 0x000000000D502004ULL)
            return &SALMON_TEXTURE;
    }
    return &FISH_TEXTURE;
}

Model* FishRenderer::createModel(const ModelDefinition& definition)
{
    return new FishModel(definition, mFishType, 0);
}
