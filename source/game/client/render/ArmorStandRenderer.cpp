#include "game/client/render/ArmorStandRenderer.hpp"

#include <cmath>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/ArmorStand.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/CustomHeadLayer.hpp"
#include "game/world/generation/ElytraLayer.hpp"
#include "game/world/generation/ItemInHandLayer.hpp"

static ResourceLocation ARMOR_STAND_LOCATION((_TEXTURE_NAME)121);

ArmorStandRenderer::ArmorStandRenderer(EntityRenderDispatcher* dispatcher)
    : LivingEntityRenderer(dispatcher, new ArmorStandModel(0.0f, ModelDefinition()), 0.0f)
{
    addLayer(new ArmorStandArmorLayer(this));
    addLayer(new ItemInHandLayer(this));
    addLayer(new ElytraLayer(this));
    addLayer(new CustomHeadLayer(static_cast<ArmorStandModel*>(getModel())->mHead));
}

ArmorStandRenderer::~ArmorStandRenderer()
{
}

void ArmorStandRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    boost::shared_ptr<ArmorStand> armorStand = boost::static_pointer_cast<ArmorStand>(entity);
    if (armorStand->isMarker())
    {
        mItemFrame = true;
    }
    LivingEntityRenderer::render(entity, x, y, z, yaw, partialTick);
    if (armorStand->isMarker())
    {
        mItemFrame = false;
    }
}

const ResourceLocation* ArmorStandRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &ARMOR_STAND_LOCATION;
}

void ArmorStandRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float, float rotationYaw, float partialTicks)
{
    boost::shared_ptr<ArmorStand> armorStand = boost::static_pointer_cast<ArmorStand>(entity);
    GlStateManager::rotatef(180.0f - rotationYaw, 0.0f, 1.0f, 0.0f);
    float time = static_cast<float>(armorStand->mLevel->getGameTime() - armorStand->mLastHit) + partialTicks;
    if (time < 5.0f)
    {
        GlStateManager::rotatef(std::sin(time / 1.5f * 3.1415927f) * 3.0f, 0.0f, 1.0f, 0.0f);
    }
}

bool ArmorStandRenderer::shouldShowName(boost::shared_ptr<Entity>& entity)
{
    if (!LivingEntityRenderer::shouldShowName(entity))
    {
        return false;
    }
    if (!entity->shouldShowName() && !entity->hasCustomName())
    {
        return false;
    }
    return entity == mDispatcher->mPlayer;
}

Model* ArmorStandRenderer::createModel(const ModelDefinition& definition)
{
    return new ArmorStandModel(0.0f, definition);
}

ArmorStandRenderer::ArmorStandArmorLayer::ArmorStandArmorLayer(LivingEntityRenderer* renderer)
    : HumanoidArmorLayer(renderer)
{
    createArmorModels();
}

ArmorStandRenderer::ArmorStandArmorLayer::~ArmorStandArmorLayer()
{
}

void ArmorStandRenderer::ArmorStandArmorLayer::createArmorModels()
{
    if (mModelArmorInner != NULL)
    {
        delete mModelArmorInner;
    }
    if (mModelArmor != NULL)
    {
        delete mModelArmor;
    }
    mModelArmorInner = new ArmorStandArmorModel(0.25f, 64, 32, ModelDefinition());
    mModelArmor = new ArmorStandArmorModel(0.5f, 64, 32, ModelDefinition());
}
