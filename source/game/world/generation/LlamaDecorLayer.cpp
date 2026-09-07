#include "game/world/generation/LlamaDecorLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/LlamaModel.hpp"
#include "game/client/render/LlamaRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Llama.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation LLAMA_DECOR_TEXTURES[16];

LlamaDecorLayer::LlamaDecorLayer(LlamaRenderer* renderer)
    : mRenderer(renderer)
{
    ModelDefinition modelDefinition;
    mModel = new LlamaModel(0.5f, modelDefinition);
}

LlamaDecorLayer::~LlamaDecorLayer()
{
}

void LlamaDecorLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                             float limbSwingAmount, float partialTicks, float ageInTicks,
                             float netHeadYaw, float headPitch, float scale,
                             bool bUseDisplayList)
{
    boost::shared_ptr<Llama> llama = boost::static_pointer_cast<Llama>(entity);
    if (!llama->hasSwag())
    {
        return;
    }

    mRenderer->bindTexture(&LLAMA_DECOR_TEXTURES[llama->getSwag()->getBlockData()]);
    mModel->copyPropertiesFrom(mRenderer->getModel());
    boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
    mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw,
                   headPitch, scale, bUseDisplayList);
}

bool LlamaDecorLayer::colorsOnDamage()
{
    return false;
}

void LlamaDecorLayer::setupForTexturePack(TexturePack* texturePack)
{
    if (mModel != NULL)
    {
        delete mModel;
    }

    if (texturePack->HasCustomModel(mRenderer->getModelName()))
    {
        mModel = new LlamaModel(0.5f,
                                *texturePack->GetCustomModel(mRenderer->getModelName()));
    }
    else if (texturePack->HasCustomModel(mRenderer->getOriginalModelName()))
    {
        mModel = new LlamaModel(
            0.5f, *texturePack->GetCustomModel(mRenderer->getOriginalModelName()));
    }
    else
    {
        ModelDefinition modelDefinition;
        mModel = new LlamaModel(0.5f, modelDefinition);
    }
}
