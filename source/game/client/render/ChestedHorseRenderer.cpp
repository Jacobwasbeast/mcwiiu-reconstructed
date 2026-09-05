#include "game/client/render/ChestedHorseRenderer.hpp"

#include "game/client/render/ChestedHorseModel.hpp"
#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

ChestedHorseRenderer::ChestedHorseRenderer(EntityRenderDispatcher* dispatcher, float shadowRadius)
    : AbstractHorseRenderer(dispatcher, new ChestedHorseModel(ModelDefinition()), shadowRadius)
{
}

ChestedHorseRenderer::~ChestedHorseRenderer()
{
}

const ResourceLocation* ChestedHorseRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    static ResourceLocation donkeyTexture;
    static ResourceLocation muleTexture;

    unsigned long long type = entity->GetType();
    if (type == ENTITY_TYPE_MULE)
        return &muleTexture;
    if (type == ENTITY_TYPE_DONKEY)
        return &donkeyTexture;
    return EntityRenderer::getTextureLocation(entity);
}

Model* ChestedHorseRenderer::createModel(const ModelDefinition& definition)
{
    return new ChestedHorseModel(definition);
}
