#include "game/client/render/PaintingRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/Painting.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/Level.hpp"

PaintingRenderer::PaintingRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher), mRandom(new Random())
{
}

PaintingRenderer::~PaintingRenderer()
{
    delete mRandom;
}

void PaintingRenderer::setBrightness(boost::shared_ptr<Painting> painting, float x, float z)
{
    Painting* value = painting.get();

    int lightX = (int)value->mX;
    int lightY = (int)(value->mY + (double)(z * 0.0625F));
    int lightZ = (int)value->mZ;

    const Direction* direction = value->getDirection();
    if (direction == Direction::NORTH)
        lightX = (int)(value->mX + (double)(x * 0.0625F));
    if (direction == Direction::WEST)
        lightZ = (int)(value->mZ - (double)(x * 0.0625F));
    if (direction == Direction::SOUTH)
        lightX = (int)(value->mX - (double)(x * 0.0625F));
    if (direction == Direction::EAST)
        lightZ = (int)(value->mZ + (double)(x * 0.0625F));

    BlockPos pos(lightX, lightY, lightZ);
    Level* level = value->getCommandSenderWorld();
    int light = level->getLightColor(pos, 0, -1);
    GlStateManager::multiTexCoord2f((float)(light & 65535), (float)((unsigned int)light >> 16));
    GlStateManager::color3f(1.0F, 1.0F, 1.0F);
}

void PaintingRenderer::renderPainting(boost::shared_ptr<Painting> painting, int width, int height,
                                      int offsetX, int offsetY)
{
    const float tileSize = 0.0625F;
    const float halfWidth = (float)width * -0.5F;
    const float halfHeight = (float)height * -0.5F;
    const float depth = 0.5F;
    const int columns = (width + 15) / 16;
    const int rows = (height + 15) / 16;
    int column;
    int row;

    for (column = 0; column < columns; ++column)
    {
        for (row = 0; row < rows; ++row)
        {
            const float left = halfWidth + (float)(column * 16);
            const float right = left + 16.0F;
            const float top = halfHeight + (float)(row * 16);
            const float bottom = top + 16.0F;
            const float u0 = (float)(offsetX + width - column * 16) * tileSize;
            const float u1 = (float)(offsetX + width - (column + 1) * 16) * tileSize;
            const float v0 = (float)(offsetY + height - row * 16) * tileSize;
            const float v1 = (float)(offsetY + height - (row + 1) * 16) * tileSize;

            setBrightness(painting, (left + right) * 0.5F, (top + bottom) * 0.5F);
            BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
            builder->begin();
            builder->normal(0.0F, 0.0F, -1.0F);
            builder->vertexUV(right, bottom, -depth, u1, v0);
            builder->vertexUV(left, bottom, -depth, u0, v0);
            builder->vertexUV(left, top, -depth, u0, v1);
            builder->vertexUV(right, top, -depth, u1, v1);
            builder->normal(0.0F, 0.0F, 1.0F);
            builder->vertexUV(left, top, depth, 0.0F, 0.0F);
            builder->vertexUV(right, top, depth, 1.0F, 0.0F);
            builder->vertexUV(right, bottom, depth, 1.0F, 1.0F);
            builder->vertexUV(left, bottom, depth, 0.0F, 1.0F);
            builder->normal(-1.0F, 0.0F, 0.0F);
            builder->vertexUV(left, bottom, -depth, 0.0F, 1.0F);
            builder->vertexUV(left, bottom, depth, 1.0F, 1.0F);
            builder->vertexUV(left, top, depth, 1.0F, 0.0F);
            builder->vertexUV(left, top, -depth, 0.0F, 0.0F);
            builder->normal(1.0F, 0.0F, 0.0F);
            builder->vertexUV(right, top, -depth, 0.0F, 0.0F);
            builder->vertexUV(right, top, depth, 1.0F, 0.0F);
            builder->vertexUV(right, bottom, depth, 1.0F, 1.0F);
            builder->vertexUV(right, bottom, -depth, 0.0F, 1.0F);
            builder->normal(0.0F, 1.0F, 0.0F);
            builder->vertexUV(left, top, -depth, 0.0F, 0.0F);
            builder->vertexUV(right, top, -depth, 1.0F, 0.0F);
            builder->vertexUV(right, top, depth, 1.0F, 1.0F);
            builder->vertexUV(left, top, depth, 0.0F, 1.0F);
            builder->normal(0.0F, -1.0F, 0.0F);
            builder->vertexUV(left, bottom, depth, 0.0F, 0.0F);
            builder->vertexUV(right, bottom, depth, 1.0F, 0.0F);
            builder->vertexUV(right, bottom, -depth, 1.0F, 1.0F);
            builder->vertexUV(left, bottom, -depth, 0.0F, 1.0F);
            Tesselator::getInstance()->end();
        }
    }
}

void PaintingRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                              float yaw, float partialTick)
{
    boost::shared_ptr<Painting> painting = boost::static_pointer_cast<Painting>(entity);
    mRandom->setSeed(187LL);
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::rotatef(180.0F - yaw, 0.0F, 1.0F, 0.0F);
    GlStateManager::enableRescaleNormal();
    bindTexture(entity);
    Painting* value = painting.get();
    GlStateManager::scalef(0.0625F, 0.0625F, 0.0625F);
    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }
    renderPainting(painting, value->getWidth(), value->getHeight(), value->getWidth(), value->getHeight());
    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* PaintingRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    static ResourceLocation location;
    return &location;
}
