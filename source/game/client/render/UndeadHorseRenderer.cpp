#include "game/client/render/UndeadHorseRenderer.hpp"

#include "game/client/render/AbstractHorseModel.hpp"
#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

UndeadHorseRenderer::UndeadHorseRenderer(EntityRenderDispatcher* dispatcher)
    : AbstractHorseRenderer(dispatcher, new AbstractHorseModel(true, ModelDefinition()), 1.0F)
{
}

UndeadHorseRenderer::~UndeadHorseRenderer()
{
}

const ResourceLocation* UndeadHorseRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    static ResourceLocation skeletonHorseTexture;
    static ResourceLocation zombieHorseTexture;

    unsigned long long type = entity->GetType();
    if (type == 0x2D690002ULL)
        return &skeletonHorseTexture;
    if (type == 0x2D690003ULL)
        return &zombieHorseTexture;
    return EntityRenderer::getTextureLocation(entity);
}

Model* UndeadHorseRenderer::createModel(const ModelDefinition& definition)
{
    return new AbstractHorseModel(true, definition);
}
