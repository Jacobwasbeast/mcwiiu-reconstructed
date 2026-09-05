#include "game/client/render/EnderDragonRenderer.hpp"

#include <cmath>

#include "game/client/render/DragonModel.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/EnderDragon.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/EnderDragonDeathLayer.hpp"
#include "game/world/generation/EnderDragonEyesLayer.hpp"

static ResourceLocation ENDER_DRAGON_LOCATION;
static float beamRotation;

EnderDragonRenderer::EnderDragonRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new DragonModel(ModelDefinition()), 0.5f), mEyesLayer(NULL)
{
    mEyesLayer = new EnderDragonEyesLayer(this);
    addLayer(mEyesLayer);
    addLayer(new EnderDragonDeathLayer());
}

EnderDragonRenderer::~EnderDragonRenderer()
{
}

void EnderDragonRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float, float, float partialTicks)
{
    boost::shared_ptr<EnderDragon> dragon = boost::static_pointer_cast<EnderDragon>(entity);
    double positions[3];
    arrayWithLength<double> offsets;
    offsets.data = positions;
    offsets.length = 3;
    dragon->getLatencyPos(offsets, 7, partialTicks);
    GlStateManager::rotatef(-(float)positions[0], 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef((float)dragon->getTilt(partialTicks), 1.0f, 0.0f, 0.0f);
    GlStateManager::translatef(0.0f, 0.0f, 1.0f);
    if (dragon->mDeathTime > 0)
    {
        float progress = Mth::sqrt(((float)dragon->mDeathTime - partialTicks) / 20.0f * 1.6f);
        if (progress > 1.0f)
            progress = 1.0f;
        GlStateManager::rotatef(progress * getDispatcher()->mCameraXRot, 0.0f, 0.0f, 1.0f);
    }
}

void EnderDragonRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                      float ageInTicks, float netHeadYaw, float headPitch, float scale)
{
    boost::shared_ptr<EnderDragon> dragon = boost::static_pointer_cast<EnderDragon>(entity);
    boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
    if (dragon->mDeathTime > 0)
    {
        GlStateManager::depthFunc(3);
        GlStateManager::enableAlphaTest();
        GlStateManager::alphaFunc(4, ((float)dragon->mDeathTime - 1.0f + ageInTicks - (float)dragon->mTickCount) / 200.0f);
        bindTexture(&ENDER_DRAGON_LOCATION);
        mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);
        GlStateManager::alphaFunc(4, 0.1f);
        GlStateManager::depthFunc(2);
    }
    bindTexture(renderEntity);
    mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);
    if (dragon->mHurtTime > 0)
    {
        GlStateManager::depthFunc(2);
        GlStateManager::disableTexture();
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        GlStateManager::color4f(1.0f, 0.0f, 0.0f, 0.5f);
        mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, false);
        GlStateManager::enableTexture();
        GlStateManager::disableBlend();
        GlStateManager::depthFunc(3);
    }
}

void EnderDragonRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    boost::shared_ptr<EnderDragon> dragon = boost::static_pointer_cast<EnderDragon>(entity);
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
    if (dragon->mBeamTarget != NULL && !dragon->mBeamTarget->showsBottom())
    {
        EnderCrystal* crystal = dragon->mBeamTarget;
        double delta = 1.0 - partialTick;
        renderCrystalBeams(x, y, z, partialTick,
                           (crystal->mX - dragon->mX) * delta + dragon->mX,
                           (crystal->mY - dragon->mY) * delta + dragon->mY,
                           (crystal->mZ - dragon->mZ) * delta + dragon->mZ,
                           crystal->mTickCount, dragon->mX, dragon->mY, dragon->mZ);
    }
}

const ResourceLocation* EnderDragonRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &ENDER_DRAGON_LOCATION;
}

void EnderDragonRenderer::setupForTexturePack(TexturePack* pack)
{
    LivingEntityRenderer::setupForTexturePack(pack);
    if (getModel()->hasOverriddenParts())
    {
        removeLayer(mEyesLayer);
        return;
    }
    for (std::vector<RenderLayer*>::iterator it = mLayers.begin(); it != mLayers.end(); ++it)
    {
        if (*it == mEyesLayer)
            return;
    }
    addLayer(mEyesLayer);
}

Model* EnderDragonRenderer::createModel(const ModelDefinition& definition)
{
    return new DragonModel(definition);
}

void EnderDragonRenderer::renderCrystalBeams(double x, double y, double z, float partialTick,
                                             double targetX, double targetY, double targetZ, int time,
                                             double crystalX, double crystalY, double crystalZ)
{
    float dx = (float)(crystalX - targetX);
    float dy = (float)(crystalY - targetY);
    float dz = (float)(crystalZ - targetZ);
    float horizontal = Mth::sqrt(dx * dx + dz * dz);
    float distance = Mth::sqrt(dx * dx + dy * dy + dz * dz);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)(y + 2.0), (float)z);
    GlStateManager::rotatef((float)(-std::atan2(dz, dx) * 57.295776f - 90.0f), 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef((float)(-std::atan2(horizontal, dy) * 57.295776f - 90.0f), 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(beamRotation, 0.0f, 0.0f, 1.0f);
    beamRotation += 0.01f;
    if (beamRotation >= 1.0f)
        beamRotation = 0.0f;
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    Lighting::turnOff();
    GlStateManager::disableCull();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 6);
    GlStateManager::shadeModel(0);
    float phase = ((float)time + partialTick) * 0.005f;
    float length = distance * 0.3f - phase;
    builder->begin(1);
    for (int base = 0; base < 8; base += 4)
    {
        for (int i = 0; i < 4; ++i)
        {
            float angle = (float)(base + i) * 0.125f * 6.2831855f;
            float sine = std::sin(angle);
            float cosine = std::cos(angle);
            float u = (float)(base + i) * 0.125f;
            builder->vertexUV(sine * 0.75f * 0.2f, cosine * 0.75f * 0.2f, 0.0f, u, length);
            builder->vertexUV(sine * 0.75f, cosine * 0.75f, distance, u, -phase);
        }
    }
    builder->vertexUV(0.0f, 0.15f, 0.0f, 0.0f, length);
    builder->vertexUV(0.0f, 0.75f, distance, 0.0f, -phase);
    builder->end();
    GlStateManager::enableCull();
    GlStateManager::shadeModel(0);
    GlStateManager::disableBlend();
    GlStateManager::popMatrix();
    Lighting::turnOn();
}
