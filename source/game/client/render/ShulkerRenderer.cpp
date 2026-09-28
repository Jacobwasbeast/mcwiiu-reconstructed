#include "game/client/render/ShulkerRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ShulkerModel.hpp"
#include "game/world/generation/ShulkerHeadLayer.hpp"
#include "game/core/Culler.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Shulker.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/util/Vec3.hpp"

static ResourceLocation SHULKER_LOCATION;

ShulkerRenderer::ShulkerRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new ShulkerModel(ModelDefinition()), 0.0F)
{
    addLayer(new ShulkerHeadLayer(this));
}

ShulkerRenderer::~ShulkerRenderer()
{
}

void ShulkerRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    boost::shared_ptr<Shulker> shulker = boost::static_pointer_cast<Shulker>(entity);
    int interpolation = shulker->getClientSideTeleportInterpolation();
    if (interpolation > 0 && shulker->hasValidInterpolationPositions())
    {
        BlockPos* oldPosition = shulker->getOldAttachPosition();
        BlockPos* position = shulker->getAttachPosition();
        float progress = ((float)interpolation - partialTick) / 6.0F;
        progress *= progress;
        MobRenderer::render(entity, -((double)(position->x - oldPosition->x) * progress - x),
                            -((double)(position->y - oldPosition->y) * progress - y),
                            -((double)(position->z - oldPosition->z) * progress - z), yaw, partialTick);
        return;
    }
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

bool ShulkerRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z)
{
    if (MobRenderer::shouldRender(entity, culler, x, y, z))
        return true;

    boost::shared_ptr<Shulker> shulker = boost::static_pointer_cast<Shulker>(entity);
    if (shulker->getClientSideTeleportInterpolation() <= 0 || !shulker->hasValidInterpolationPositions())
        return false;

    BlockPos* oldPosition = shulker->getOldAttachPosition();
    BlockPos* position = shulker->getAttachPosition();
    Vec3* oldPositionVector = Vec3::newPermanent(oldPosition->x, oldPosition->y, oldPosition->z);
    Vec3* positionVector = Vec3::newPermanent(position->x, position->y, position->z);
    AABB* box = AABB::newPermanent(oldPositionVector->x, oldPositionVector->y, oldPositionVector->z,
                                    positionVector->x, positionVector->y, positionVector->z);
    return culler->isVisible(box);
}

const ResourceLocation* ShulkerRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &SHULKER_LOCATION;
}

void ShulkerRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float bodyRot, float partialTick)
{
    LivingEntityRenderer::setupRotations(entity, ageInTicks, bodyRot, partialTick);
    boost::shared_ptr<Shulker> shulker = boost::static_pointer_cast<Shulker>(entity);
    const Direction* face = shulker->getAttachFace();
    if (face == Direction::EAST)
    {
        GlStateManager::translatef(0.5F, 0.5F, 0.0F);
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager::rotatef(90.0F, 0.0F, 0.0F, 1.0F);
    }
    else if (face == Direction::WEST)
    {
        GlStateManager::translatef(-0.5F, 0.5F, 0.0F);
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager::rotatef(-90.0F, 0.0F, 0.0F, 1.0F);
    }
    else if (face == Direction::NORTH)
    {
        GlStateManager::translatef(0.0F, 0.5F, -0.5F);
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
    }
    else if (face == Direction::SOUTH)
    {
        GlStateManager::translatef(0.0F, 0.5F, 0.5F);
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager::rotatef(180.0F, 0.0F, 0.0F, 1.0F);
    }
    else if (face == Direction::UP)
    {
        GlStateManager::translatef(0.0F, 1.0F, 0.0F);
        GlStateManager::rotatef(180.0F, 1.0F, 0.0F, 0.0F);
    }
}

void ShulkerRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    GlStateManager::scalef(0.999F, 0.999F, 0.999F);
}

void ShulkerRenderer::renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount, float partialTick, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool overlay)
{
    if (!mModel->hasOverriddenParts())
        LivingEntityRenderer::renderLayers(entity, limbSwing, limbSwingAmount, partialTick, ageInTicks, netHeadYaw, headPitch, scale, overlay);
}

void ShulkerRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale)
{
    if (!isVisible(entity) && entity->isInvisibleTo(boost::static_pointer_cast<Player>(Minecraft::GetInstance()->mPlayer)))
    {
        boost::shared_ptr<Entity> renderEntity = entity;
        if (!bindTexture(renderEntity))
            return;
        ShulkerModel* model = static_cast<ShulkerModel*>(getModel(entity));
        model->renderWhenInvisible(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);
        return;
    }
    LivingEntityRenderer::renderModel(entity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale);
}

Model* ShulkerRenderer::createModel(const ModelDefinition& definition)
{
    return new ShulkerModel(definition);
}
