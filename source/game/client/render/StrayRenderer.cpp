#include "game/client/render/StrayRenderer.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SkeletonModel.hpp"
#include "game/client/render/RenderLayer.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/CustomHeadLayer.hpp"
#include "game/world/generation/StrayClothingLayer.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation STRAY_LOCATION;

StrayRenderer::StrayRenderer(EntityRenderDispatcher* dispatcher)
    : SkeletonRenderer(dispatcher, new SkeletonModel(SkeletonModel::eSkeletonType_Stray, ModelDefinition()))
{
    mStrayClothingLayer = new StrayClothingLayer(this);
    addLayer(mStrayClothingLayer);
}

StrayRenderer::~StrayRenderer()
{
}

std::wstring StrayRenderer::getModelName()
{
    return L"skeleton_stray";
}

const ResourceLocation* StrayRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &STRAY_LOCATION;
}

void StrayRenderer::renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float ageInTicks, float netHeadYaw, float headPitch, float scale, float partialTick,
                                 bool overlay)
{
    bool invisibleToPlayer = false;
    if (!mModel->hasOverriddenParts())
    {
        boost::shared_ptr<Player> player = Minecraft::GetInstance()->mPlayer;
        if (player != NULL)
            invisibleToPlayer = entity->isInvisibleTo(player);
    }

    std::vector<RenderLayer*>::iterator it = mLayers.begin();
    while (it != mLayers.end())
    {
        RenderLayer* layer = *it;
        if ((mModel->hasOverriddenParts() && !invisibleToPlayer) || layer != mStrayClothingLayer)
        {
            bool skipLayer = false;
            if (entity->isInvisible())
            {
                boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
                if (player != NULL && player->SpectateAsFloatingHead() && dynamic_cast<CustomHeadLayer*>(layer) == NULL)
                    skipLayer = true;
            }

            if (!skipLayer)
            {
                bool teardown = setupOverlayColor(entity, partialTick, layer->colorsOnDamage());
                layer->render(entity, limbSwing, limbSwingAmount, partialTick, ageInTicks, netHeadYaw, headPitch,
                              scale, overlay);
                if (teardown)
                    teardownOverlayColor();
            }
        }
        ++it;
    }
}

Model* StrayRenderer::createModel(const ModelDefinition& definition)
{
    return new SkeletonModel(SkeletonModel::eSkeletonType_Stray, definition);
}
