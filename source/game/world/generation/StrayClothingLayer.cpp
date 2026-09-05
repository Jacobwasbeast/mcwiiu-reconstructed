#include "game/world/generation/StrayClothingLayer.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/ZombieModel.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation STRAY_CLOTHING_LOCATION;

StrayClothingLayer::StrayClothingLayer(LivingEntityRenderer* renderer)
    : mRenderer(renderer)
{
    ModelDefinition modelDefinition;
    mModel = new ZombieModel(0.25f, false, modelDefinition);
    mOriginalModel = NULL;
}

StrayClothingLayer::~StrayClothingLayer()
{
}

void StrayClothingLayer::render(boost::shared_ptr<LivingEntity>& entity,
                                float limbSwing, float limbSwingAmount,
                                float partialTicks, float ageInTicks,
                                float netHeadYaw, float headPitch, float scale,
                                bool bUseDisplayList)
{
    mModel->copyPropertiesFrom(mRenderer->getModel(entity));
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    mRenderer->bindTexture(&STRAY_CLOTHING_LOCATION);

    boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
    mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw,
                   headPitch, scale, bUseDisplayList);
}

bool StrayClothingLayer::colorsOnDamage()
{
    return true;
}

void StrayClothingLayer::setupForTexturePack(TexturePack* pTexturePack)
{
    if (mOriginalModel == NULL)
    {
        mOriginalModel = mModel;
    }

    if (mModel != mOriginalModel)
    {
        if (mModel != NULL)
        {
            delete mModel;
        }
        mModel = NULL;
    }

    const std::wstring modelName = L"stray_armor";
    if (!pTexturePack->HasCustomModel(modelName))
    {
        mModel = mOriginalModel;
    }
    else
    {
        mModel = new ZombieModel(*pTexturePack->GetCustomModel(modelName));
    }
}
