#include "game/client/render/ChestedHorseRenderer.hpp"

#include "game/client/render/ChestedHorseModel.hpp"
#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation DONKEY_LOCATION(static_cast<_TEXTURE_NAME>(88));
static ResourceLocation MULE_LOCATION(static_cast<_TEXTURE_NAME>(102));

ChestedHorseRenderer::ChestedHorseRenderer(EntityRenderDispatcher* dispatcher, float shadowRadius)
    : AbstractHorseRenderer(dispatcher, new ChestedHorseModel(ModelDefinition()), shadowRadius)
{
}

ChestedHorseRenderer::~ChestedHorseRenderer()
{
}

const ResourceLocation* ChestedHorseRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    unsigned long long type = entity->GetType();
    if (type == ENTITY_TYPE_MULE)
        return &MULE_LOCATION;
    if (type == ENTITY_TYPE_DONKEY)
        return &DONKEY_LOCATION;
    return EntityRenderer::getTextureLocation(entity);
}

Model* ChestedHorseRenderer::createModel(const ModelDefinition& definition)
{
    return new ChestedHorseModel(definition);
}
