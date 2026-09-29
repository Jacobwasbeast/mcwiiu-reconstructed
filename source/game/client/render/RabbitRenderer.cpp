#include "game/client/render/RabbitRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/RabbitModel.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Rabbit.hpp"

static ResourceLocation RABBIT_TEXTURES[8] = {
    ResourceLocation(TN_MOB_RABBIT_BROWN),
    ResourceLocation(TN_MOB_RABBIT_WHITE),
    ResourceLocation(TN_MOB_RABBIT_BLACK),
    ResourceLocation(TN_MOB_RABBIT_GOLD),
    ResourceLocation(TN_MOB_RABBIT_SALT),
    ResourceLocation(TN_MOB_RABBIT_WHITE_SPLOTCHED),
    ResourceLocation(TN_MOB_RABBIT_TOAST),
    ResourceLocation(TN_MOB_RABBIT_CAERBANNOG)
};

RabbitRenderer::RabbitRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new RabbitModel(ModelDefinition()), 0.3f)
{
}

RabbitRenderer::~RabbitRenderer()
{
}

const ResourceLocation* RabbitRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    if (entity->getName() == L"Toast")
        return &RABBIT_TEXTURES[6];

    boost::shared_ptr<Rabbit> rabbit = boost::static_pointer_cast<Rabbit>(entity);
    unsigned int type = rabbit->getRabbitType();

    if (type == 1)
        return &RABBIT_TEXTURES[1];
    if (type == 2)
        return &RABBIT_TEXTURES[2];
    if (type == 3)
        return &RABBIT_TEXTURES[5];
    if (type == 4)
        return &RABBIT_TEXTURES[3];
    if (type == 5)
        return &RABBIT_TEXTURES[4];
    if (type == 99)
        return &RABBIT_TEXTURES[7];
    return &RABBIT_TEXTURES[0];
}

Model* RabbitRenderer::createModel(const ModelDefinition& definition)
{
    return new RabbitModel(definition);
}
