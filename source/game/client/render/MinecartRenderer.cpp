#include "game/client/render/MinecartRenderer.hpp"

#include <cmath>

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/MinecartModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Minecart.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/Mth.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/Block.hpp"

MinecartRenderer::MinecartRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
    mShadowRadius = 0.5F;
    ModelDefinition definition;
    mModel = new MinecartModel(definition);
}

MinecartRenderer::~MinecartRenderer()
{
}

void MinecartRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                              float yaw, float partialTick)
{
    boost::shared_ptr<Minecart> minecart = boost::static_pointer_cast<Minecart>(entity);
    GlStateManager::pushMatrix();
    EntityRenderer::postRender(entity, x, y, z, yaw, partialTick, false);

    int id = minecart->getId();
    unsigned int hash = (unsigned int)(id * 0x1D66F537) * (unsigned int)(id * 0x1D66F537) * 0x5CB26D1U
                      + (unsigned int)(id * -0x1361C0D1);
    float offsetX = (float)((hash >> 16) & 7) * 0.125F;
    float offsetY = (float)((hash >> 20) & 7) * 0.125F;
    float offsetZ = (float)((hash >> 24) & 7) * 0.125F;
    GlStateManager::translatef(offsetX, offsetY, offsetZ);

    double renderX = minecart->mXo + (minecart->mX - minecart->mXo) * partialTick;
    double renderY = minecart->mYo + (minecart->mY - minecart->mYo) * partialTick;
    double renderZ = minecart->mZo + (minecart->mZ - minecart->mZo) * partialTick;
    Vec3* trackPosition = minecart->getPos(renderX, renderY, renderZ);
    if (trackPosition != NULL)
    {
        Vec3* frontPosition = minecart->getPosOffs(renderX, renderY, renderZ, 0.3);
        Vec3* backPosition = minecart->getPosOffs(renderX, renderY, renderZ, -0.3);
        if (frontPosition == NULL)
            frontPosition = trackPosition;
        if (backPosition == NULL)
            backPosition = trackPosition;
        x += trackPosition->x - renderX;
        y += (frontPosition->y + backPosition->y) * 0.5 - renderY;
        z += trackPosition->z - renderZ;
        Vec3 direction(-frontPosition->x + backPosition->x,
                       -frontPosition->y + backPosition->y,
                       -frontPosition->z + backPosition->z);
        if (direction.length() != 0.0)
        {
            direction.normalize();
            yaw = (float)(std::atan2(direction.z, direction.x) * 180.0 / 3.14159265358979323846);
            float pitch = (float)(std::atan(direction.y) * 180.0 / 3.14159265358979323846);
            GlStateManager::translatef((float)x, (float)y + 0.375F, (float)z);
            GlStateManager::rotatef(180.0F - yaw, 0.0F, 1.0F, 0.0F);
            GlStateManager::rotatef(-pitch, 1.0F, 0.0F, 0.0F);
        }
        else
            GlStateManager::translatef((float)x, (float)y + 0.375F, (float)z);
    }
    else
        GlStateManager::translatef((float)x, (float)y + 0.375F, (float)z);

    float hurtTime = (float)minecart->getHurtTime() - partialTick;
    float damage = minecart->getDamage() - partialTick;
    if (damage < 0.0F)
        damage = 0.0F;
    if (hurtTime > 0.0F)
    {
        GlStateManager::rotatef(Mth::sin(hurtTime) * hurtTime * damage / 10.0F * minecart->getHurtDir(),
                                 1.0F, 0.0F, 0.0F);
    }

    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }

    const BlockState* state = minecart->getDisplayBlockState();
    if (state != NULL && state->getRenderShape() != BLOCK_RENDER_INVISIBLE)
        renderMinecartContents(minecart, partialTick, state);

    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
    static_cast<MinecartModel*>(mModel)->render(entity, 0.0F, 0.0F, -0.1F, 0.0F, 0.0F,
                                                0.0625F, false);
    GlStateManager::popMatrix();

    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }

    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* MinecartRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    static ResourceLocation location;
    return &location;
}

void MinecartRenderer::renderMinecartContents(boost::shared_ptr<Minecart>, float,
                                              const BlockState* state)
{
    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.0F, -0.5F, 0.0F);
    Minecraft::GetInstance()->getBlockRenderer()->renderSingleBlock(state, 1.0F, 1.0F, true);
    GlStateManager::popMatrix();
}

Model* MinecartRenderer::createModel(const ModelDefinition& definition)
{
    return new MinecartModel(definition);
}
