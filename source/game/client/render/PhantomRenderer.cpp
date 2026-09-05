#include "game/client/render/PhantomRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/PhantomModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation PHANTOM_LOCATION;

PhantomRenderer::PhantomRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new PhantomModel(ModelDefinition()), 0.5F)
{
}

PhantomRenderer::~PhantomRenderer()
{
}

const ResourceLocation* PhantomRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &PHANTOM_LOCATION;
}

void PhantomRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTick)
{
    LivingEntityRenderer::scale(entity, partialTick);
    GlStateManager::scalef(0.75F, 0.75F, 0.75F);
    GlStateManager::translatef(0.0F, 1.035F, 0.0F);
}

void PhantomRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks,
                                     float rotationYaw, float partialTicks)
{
    LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);
    GlStateManager::rotatef(entity->mYRot, 1.0F, 0.0F, 0.0F);
}

Model* PhantomRenderer::createModel(const ModelDefinition& definition)
{
    return new PhantomModel(definition);
}

void PhantomRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                  float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                  float headPitch, float scale)
{
    bool translucent = false;
    if (!isVisible(entity))
    {
        boost::shared_ptr<Player> player = Minecraft::GetInstance()->mPlayer;
        if (entity->isInvisibleTo(player))
            return;
        translucent = true;
        GlStateManager::setProfile();
    }

    Model* model = getModel(entity);
    model->render(entity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale,
                  translucent);

    if (translucent)
        GlStateManager::unsetProfile();
}
