#include "game/client/render/IllusionIllagerRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/IllagerModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/IllusionIllager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/SpellcasterIllager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/biome/arrayWithLength.hpp"

static ResourceLocation ILLUSION_ILLAGER_LOCATION(TN_MOB_ILLAGER_ILLUSIONIST);

IllusionIllagerRenderer::IllusionIllagerRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new IllagerModel(0.0f, 0.0f, 64, 64), 0.5f)
{
    addLayer(new IllusionIllagerRenderer_ItemInHandLayer(this));
}

const ResourceLocation* IllusionIllagerRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &ILLUSION_ILLAGER_LOCATION;
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
    }
    else
    {
        arrayWithLength<Vec3*> offsets = boost::static_pointer_cast<IllusionIllager>(entity)->getIllusionOffsets(partialTick);
        boost::shared_ptr<LivingEntity> livingEntity = boost::static_pointer_cast<LivingEntity>(entity);
        float bob = getBob(livingEntity, partialTick);
        for (unsigned int index = 0; index < offsets.length; ++index)
        {
            Vec3* offset = offsets.data[index];
            float phase = (float)index;
            MobRenderer::render(entity,
                                Mth::cos(phase + bob * 0.5f) * 0.025 + x + offset->x,
                                Mth::cos(phase + bob * 0.75f) * 0.0125 + y + offset->y,
                                Mth::cos(phase + bob * 0.7f) * 0.025 + z + offset->z,
                                yaw, partialTick);
        }
        delete offsets.data;
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
    if (boost::static_pointer_cast<IllusionIllager>(entity)->isCastingSpell() ||
        boost::static_pointer_cast<IllusionIllager>(entity)->isAggressive())
        ItemInHandLayer::render(entity, limbSwing, limbSwingAmount, partialTicks, ageInTicks, netHeadYaw,
                                headPitch, scale, bUseDisplayList);
}

void IllusionIllagerRenderer::IllusionIllagerRenderer_ItemInHandLayer::translateToHand(const HumanoidArm* arm)
{
    IllagerModel* model = static_cast<IllagerModel*>(mLivingEntityRenderer->getModel());
    model->getArm(arm)->translateTo(0.0625f);
}
