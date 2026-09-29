#include "game/client/render/StrayRenderer.hpp"
#include "game/core/EntityType.hpp"

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
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/StrayClothingLayer.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation STRAY_LOCATION(TN_MOB_SKELETON_STRAY);

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
    return L"skeleton.stray";
}

const ResourceLocation* StrayRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &STRAY_LOCATION;
}

void StrayRenderer::renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTick, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                 bool overlay)
{
    bool visible = isVisible(entity);
    bool invisibleToPlayer = false;
    if (!visible)
        invisibleToPlayer = !entity->isInvisibleTo(Minecraft::GetInstance()->mPlayer);

    std::vector<RenderLayer*>::iterator it = mLayers.begin();
    while (it != mLayers.end())
    {
        RenderLayer* layer = *it;
        if ((visible && !invisibleToPlayer) || layer != mStrayClothingLayer)
        {
            bool skipLayer = false;
            if ((entity->GetType() & EntityType::PLAYER) == EntityType::PLAYER
                && boost::static_pointer_cast<Player>(entity)->isSpectator()
                && boost::static_pointer_cast<Player>(entity)->SpectateAsFloatingHead()
                && (layer == NULL || dynamic_cast<StrayClothingLayer*>(layer) == NULL))
            {
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
