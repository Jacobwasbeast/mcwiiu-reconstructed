#include "game/client/render/ThrownTridentRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/TridentModel.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

ThrownTridentRenderer::ThrownTridentRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
    ModelDefinition definition;
    mModel = new TridentModel(definition);
}

ThrownTridentRenderer::~ThrownTridentRenderer()
{
}

void ThrownTridentRenderer::renderFoil(boost::shared_ptr<Entity>& entity)
{
    GlStateManager::depthFunc(2);
    GlStateManager::disableLighting();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(2, 1);
    GlStateManager::matrixMode(2);
    GlStateManager::pushMatrix();
    GlStateManager::color4f(0.38F, 0.19F, 0.608F, 1.0F);
    GlStateManager::scalef(0.125F, 0.125F, 0.125F);
    GlStateManager::translatef((float)(std::fmod((double)0.0, 3000.0) / 8.0 * -50.0), 256.0F, 256.0F);
    GlStateManager::rotatef(4873.0F, 256.0F, 256.0F, 1.0F);
    if (mModel != NULL)
        mModel->render(entity, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, true);
    GlStateManager::popMatrix();
    GlStateManager::pushMatrix();
    GlStateManager::color4f(0.38F, 0.19F, 0.608F, 1.0F);
    GlStateManager::scalef(0.125F, 0.125F, 0.125F);
    GlStateManager::translatef((float)(-std::fmod((double)0.0, 4873.0) / 10.0 * -50.0), 256.0F, 256.0F);
    GlStateManager::rotatef(10.0F, 256.0F, 256.0F, 1.0F);
    if (mModel != NULL)
        mModel->render(entity, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, true);
    GlStateManager::matrixMode(2);
    GlStateManager::popMatrix();
    GlStateManager::matrixMode(0);
    GlStateManager::disableBlend();
    GlStateManager::enableLighting();
    GlStateManager::depthFunc(3);
}

double ThrownTridentRenderer::lerp(double first, double second, double amount)
{
    return (second - first) * amount + first;
}

void ThrownTridentRenderer::renderLeash(boost::shared_ptr<ThrownTrident>& entity, double x, double y, double z,
                                        float yaw, float partialTick)
{
    if (entity->isNoPhysics())
    {
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        GlStateManager::disableTexture();
        GlStateManager::disableLighting();
        GlStateManager::disableCull();
        builder->begin(1);
        float length = 0.5F + partialTick * 0.5F;
        float phase = yaw * 0.017453292F;
        int segment;
        for (segment = 0; segment < 16; ++segment)
        {
            float progress = (float)segment / 16.0F;
            float next = (float)(segment + 1) / 16.0F;
            float curve = (progress * 2.0F - 1.0F) * (progress * 2.0F - 1.0F) - 1.0F;
            float nextCurve = (next * 2.0F - 1.0F) * (next * 2.0F - 1.0F) - 1.0F;
            float px = (float)x + std::sin(phase) * curve * 0.5F;
            float pz = (float)z + std::cos(phase) * curve * 0.5F;
            float nx = (float)x + std::sin(phase) * nextCurve * 0.5F;
            float nz = (float)z + std::cos(phase) * nextCurve * 0.5F;
            float py = (float)y + progress * length;
            float ny = (float)y + next * length;
            float color = (segment & 1) == 0 ? 0.3F : 0.37F;
            builder->color(color, color, color, 1.0F);
            builder->vertex(px, py, pz);
            builder->vertex(nx, ny, nz);
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
        GlStateManager::rotatef(entity->mXRotO + (entity->mXRot - entity->mXRotO) * partialTick - 90.0F,
                                0.0F, 1.0F, 0.0F);
        GlStateManager::rotatef(entity->mYRotO + (entity->mYRot - entity->mYRotO) * partialTick + 90.0F,
                                1.0F, 0.0F, 0.0F);
        if (mModel != NULL)
            mModel->render(entity, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0625F, true);
        GlStateManager::popMatrix();
        GlStateManager::enableLighting();
        bindTexture(entity);
    }
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* ThrownTridentRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    static ResourceLocation location((_TEXTURE_NAME)230);
    (void)entity;
    return &location;
}

Model* ThrownTridentRenderer::createModel(const ModelDefinition& definition)
{
    return new TridentModel(definition);
}
