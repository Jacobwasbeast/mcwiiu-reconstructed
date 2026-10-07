#include "game/client/render/ThrownTridentRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/TridentModel.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation TRIDENT_LOCATION(TN_ITEM_TRIDENT);

ThrownTridentRenderer::ThrownTridentRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
    mModel = new TridentModel();
}

ThrownTridentRenderer::~ThrownTridentRenderer()
{
}

void ThrownTridentRenderer::renderFoil(boost::shared_ptr<Entity>& entity)
{
    Minecraft::GetInstance()->mGameRenderer->resetFogColor(true);
    GlStateManager::depthFunc(2);
    GlStateManager::disableLighting();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(2, 1);
    Minecraft::GetInstance()->mTextures->bindTexture(&ItemRenderer::ENCHANT_GLINT_LOCATION);
    GlStateManager::matrixMode(2);
    GlStateManager::pushMatrix();
    GlStateManager::color4f(0.38F, 0.19F, 0.608F, 1.0F);
    GlStateManager::scalef(0.125F, 0.125F, 0.125F);
    float offset = (float)(Minecraft::currentTimeMillis() % 3000LL);
    GlStateManager::translatef(offset / 3000.0F * 8.0F, 0.0F, 0.0F);
    GlStateManager::rotatef(-50.0F, 0.0F, 0.0F, 1.0F);
    mModel->render(entity, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, true);
    GlStateManager::popMatrix();
    GlStateManager::pushMatrix();
    GlStateManager::color4f(0.38F, 0.19F, 0.608F, 1.0F);
    GlStateManager::scalef(0.125F, 0.125F, 0.125F);
    offset = (float)(Minecraft::currentTimeMillis() % 4873LL);
    GlStateManager::translatef(-(offset / 4873.0F * 8.0F), 0.0F, 0.0F);
    GlStateManager::rotatef(10.0F, 0.0F, 0.0F, 1.0F);
    mModel->render(entity, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, true);
    GlStateManager::matrixMode(2);
    GlStateManager::popMatrix();
    GlStateManager::matrixMode(0);
    GlStateManager::disableBlend();
    GlStateManager::enableLighting();
    GlStateManager::depthFunc(3);
    Minecraft::GetInstance()->mGameRenderer->resetFogColor(false);
}

double ThrownTridentRenderer::lerp(double first, double second, double amount)
{
    return (second - first) * amount + first;
}

void ThrownTridentRenderer::renderLeash(boost::shared_ptr<ThrownTrident>& entity, double x, double y, double z,
                                        float yaw, float partialTick)
{
    boost::shared_ptr<Entity> owner = entity->mOwner;
    if (owner && entity->isNoPhysics())
    {
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        double ownerYaw = lerp(owner->mYRotO, owner->mYRot, partialTick * 0.5F);
        double cosYaw = cos(ownerYaw * Mth::DEG_TO_RAD);
        double sinYaw = sin(ownerYaw * Mth::DEG_TO_RAD);
        double ownerX = lerp(owner->mXo, owner->mX, partialTick);
        double ownerY = lerp(owner->mYo + owner->getEyeHeight() * 0.8,
                             owner->mY + owner->getEyeHeight() * 0.8, partialTick);
        double ownerZ = lerp(owner->mZo, owner->mZ, partialTick);
        double xFactor = cosYaw - sinYaw;
        double zFactor = sinYaw + cosYaw;
        float dx = (float)(ownerX - lerp(entity->mXo, entity->mX, partialTick));
        float dy = (float)(ownerY - lerp(entity->mYo, entity->mY, partialTick));
        float dz = (float)(ownerZ - lerp(entity->mZo, entity->mZ, partialTick));
        float distance = Mth::sqrt((double)(dz * dz + dx * dx + dy * dy));
        int seed = entity->getId() + entity->mTickCount;
        double sag = distance / 30.0;
        if (0.5 - sag < 0.0)
            sag = 0.5;
        double phase = (float)((float)seed + partialTick) * -0.1;
        GlStateManager::disableTexture();
        GlStateManager::disableLighting();
        GlStateManager::disableCull();
        int shift = 7 - seed % 7;
        builder->begin(1);
        for (int i = 0; i <= 37; ++i)
        {
            double t = i / 37.0;
            double u = t + t - 1.0;
            float angle = (float)(t * 3.1415927F * 8.0 + phase);
            float bright = 1.0F - (float)((i + shift) % 7) / 7.0F;
            float dim = 1.0F - bright;
            double curve = -(u * u - 1.0) * sag;
            double px = Mth::sin(angle) * xFactor * curve + dx * t + x;
            double py = Mth::cos(angle) * 0.02 + dy * t + y + curve + 0.1;
            double pz = Mth::sin(angle) * zFactor * curve + dz * t + z;
            builder->color(0.87F * bright + 0.3F * dim, 0.91F * bright + 0.6F * dim,
                           0.85F * bright + 0.5F * dim, 1.0F);
            builder->vertex((float)px, (float)py, (float)pz);
            builder->vertex((float)(px + 0.1 * curve), (float)(py + 0.1 * curve), (float)pz);
        }
        builder->end();
        builder->begin(1);
        for (int i = 0; i <= 37; ++i)
        {
            double t = i / 37.0;
            double u = t + t - 1.0;
            double curve = -(u * u - 1.0) * sag;
            float bright = 1.0F - (float)((i + shift) % 7) / 7.0F;
            float angle = (float)(t * 3.1415927F * 8.0 + phase);
            float dim = 1.0F - bright;
            double px = Mth::sin(angle) * xFactor * curve + dx * t + x;
            double py = Mth::cos(angle) * 0.01 + dy * t + y + curve + 0.1;
            double pz = Mth::sin(angle) * zFactor * curve + dz * t + z;
            builder->color(0.87F * bright + 0.3F * dim, 0.91F * bright + 0.6F * dim,
                           0.85F * bright + 0.5F * dim, 1.0F);
            builder->vertex((float)px, (float)py, (float)pz);
            builder->vertex((float)(px + 0.1 * curve), (float)py, (float)(pz + 0.1 * curve));
        }
        builder->end();
        GlStateManager::enableLighting();
        GlStateManager::enableTexture();
        GlStateManager::enableCull();
    }
}

void ThrownTridentRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                   float yaw, float partialTick)
{
    if (entity->GetType() == EntityType::THROWN_TRIDENT)
    {
        bindTexture(entity);
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
        GlStateManager::pushMatrix();
        GlStateManager::disableLighting();
        GlStateManager::translatef((float)x, (float)y, (float)z);
        GlStateManager::rotatef(entity->mYRotO + (entity->mYRot - entity->mYRotO) * partialTick - 90.0F,
                                0.0F, 1.0F, 0.0F);
        GlStateManager::rotatef(entity->mXRotO + (entity->mXRot - entity->mXRotO) * partialTick + 90.0F,
                                0.0F, 0.0F, 1.0F);
        mModel->render(entity, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, true);
        if (boost::static_pointer_cast<ThrownTrident>(entity)->isEnchanted())
            renderFoil(entity);
        GlStateManager::popMatrix();
        GlStateManager::enableLighting();
        bindTexture(entity);
        boost::shared_ptr<ThrownTrident> trident = boost::static_pointer_cast<ThrownTrident>(entity);
        renderLeash(trident, x, y, z, yaw, partialTick);
        EntityRenderer::render(entity, x, y, z, yaw, partialTick);
    }
}

const ResourceLocation* ThrownTridentRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &TRIDENT_LOCATION;
}

Model* ThrownTridentRenderer::createModel(const ModelDefinition& definition)
{
    return new TridentModel(definition);
}
