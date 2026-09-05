#include "game/client/render/IllusionIllagerRenderer.hpp"

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/client/render/IllagerModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"

class IllusionIllager : public LivingEntity
{
public:
    virtual bool isCastingSpell();
    std::vector<Vec3*> getIllusionOffsets(float partialTick);
};

IllusionIllagerRenderer::IllusionIllagerRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new IllagerModel(0.0f, 0.0f, 64, 64, ModelDefinition()), 0.5f)
{
    addLayer(new IllusionIllagerRenderer_ItemInHandLayer(this));
}

IllusionIllagerRenderer::~IllusionIllagerRenderer()
{
}

const ResourceLocation* IllusionIllagerRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    static ResourceLocation location;
    return &location;
}

void IllusionIllagerRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    GlStateManager::scalef(0.9375f, 0.9375f, 0.9375f);
}

void IllusionIllagerRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                     float yaw, float partialTick)
{
    if (!entity->isInvisible())
    {
        MobRenderer::render(entity, x, y, z, yaw, partialTick);
        return;
    }

    boost::shared_ptr<IllusionIllager> illusion = boost::static_pointer_cast<IllusionIllager>(entity);
    std::vector<Vec3*> offsets = illusion->getIllusionOffsets(partialTick);
    for (std::vector<Vec3*>::iterator it = offsets.begin(); it != offsets.end(); ++it)
    {
        Vec3* offset = *it;
        MobRenderer::render(entity, x + offset->x, y + offset->y, z + offset->z, yaw, partialTick);
    }
}

void IllusionIllagerRenderer::renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z)
{
    LivingEntityRenderer::renderName(entity, x, y, z);
}

bool IllusionIllagerRenderer::isVisible(boost::shared_ptr<LivingEntity>&)
{
    return true;
}

Model* IllusionIllagerRenderer::createModel(const ModelDefinition& definition)
{
    return new IllagerModel(0.0f, 0.0f, 64, 64, definition);
}

IllusionIllagerRenderer::IllusionIllagerRenderer_ItemInHandLayer::IllusionIllagerRenderer_ItemInHandLayer(
    IllusionIllagerRenderer* renderer)
    : ItemInHandLayer(renderer)
{
}

IllusionIllagerRenderer::IllusionIllagerRenderer_ItemInHandLayer::~IllusionIllagerRenderer_ItemInHandLayer()
{
}

void IllusionIllagerRenderer::IllusionIllagerRenderer_ItemInHandLayer::render(
    boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount, float partialTicks,
    float ageInTicks, float netHeadYaw, float headPitch, float scale, bool bUseDisplayList)
{
    boost::shared_ptr<IllusionIllager> illusion = boost::static_pointer_cast<IllusionIllager>(entity);
    bool renderItems = illusion->isCastingSpell();
    if (!renderItems)
    {
        renderItems = entity->isInvisible();
    }
    if (renderItems)
    {
        ItemInHandLayer::render(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw,
                                headPitch, scale, bUseDisplayList);
    }
}

void IllusionIllagerRenderer::IllusionIllagerRenderer_ItemInHandLayer::translateToHand(const HumanoidArm* arm)
{
    ItemInHandLayer::translateToHand(arm);
}
