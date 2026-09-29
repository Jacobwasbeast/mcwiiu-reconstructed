#include "game/client/render/VindicationIllagerRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/IllagerModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/VindicationIllager.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation vindicationIllagerLocation(TN_MOB_ILLAGER_VINDICATOR);

VindicationIllagerRenderer::VindicationIllagerRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new IllagerModel(0.0f, 0.0f, 64, 64, ModelDefinition()), 0.5f)
{
    addLayer(new VindicationIllagerRenderer_AnonymousItemInHandLayer(this));
}

VindicationIllagerRenderer::~VindicationIllagerRenderer()
{
}

void VindicationIllagerRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                        float yaw, float partialTick)
{
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* VindicationIllagerRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &vindicationIllagerLocation;
}

void VindicationIllagerRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(0.9375f, 0.9375f, 0.9375f);
}

Model* VindicationIllagerRenderer::createModel(const ModelDefinition& definition)
{
    return new IllagerModel(0.0f, 0.0f, 64, 64, definition);
}

VindicationIllagerRenderer::VindicationIllagerRenderer_AnonymousItemInHandLayer::
    VindicationIllagerRenderer_AnonymousItemInHandLayer(VindicationIllagerRenderer* renderer)
    : ItemInHandLayer(renderer)
{
}

VindicationIllagerRenderer::VindicationIllagerRenderer_AnonymousItemInHandLayer::
    ~VindicationIllagerRenderer_AnonymousItemInHandLayer()
{
}

void VindicationIllagerRenderer::VindicationIllagerRenderer_AnonymousItemInHandLayer::render(
    boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount, float partialTicks,
    float ageInTicks, float netHeadYaw, float headPitch, float scale, bool bUseDisplayList)
{
    boost::shared_ptr<VindicationIllager> vindicationIllager =
        boost::static_pointer_cast<VindicationIllager>(entity);
    if (vindicationIllager->isAgressive())
    {
        ItemInHandLayer::render(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw,
                                headPitch, scale, bUseDisplayList);
    }
}

void VindicationIllagerRenderer::VindicationIllagerRenderer_AnonymousItemInHandLayer::translateToHand(
    const HumanoidArm* arm)
{
    IllagerModel* model = static_cast<IllagerModel*>(mLivingEntityRenderer->getModel());
    model->getArm(arm)->translateTo(0.0625f);
}
