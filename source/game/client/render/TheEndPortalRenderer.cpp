#include "game/client/render/TheEndPortalRenderer.hpp"

#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Direction.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/System.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/TheEndPortalBlockEntity.hpp"

#include <boost/shared_ptr.hpp>

static ResourceLocation END_SKY_LOCATION((_TEXTURE_NAME)74);
static ResourceLocation END_PORTAL_LOCATION((_TEXTURE_NAME)75);

static Random RANDOM(31100LL);

static FloatBuffer* MODELVIEW = MemoryTracker::createFloatBuffer(16);
static FloatBuffer* PROJECTION = MemoryTracker::createFloatBuffer(16);

TheEndPortalRenderer::TheEndPortalRenderer()
{
    mBuffer = MemoryTracker::createFloatBuffer(16);
}

TheEndPortalRenderer::~TheEndPortalRenderer()
{
}

FloatBuffer* TheEndPortalRenderer::getBuffer(float x, float y, float z, float w)
{
    mBuffer->clear();
    mBuffer->put(x);
    mBuffer->put(y);
    mBuffer->put(z);
    mBuffer->put(w);
    mBuffer->flip();

    return mBuffer;
}

void TheEndPortalRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a, int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    boost::shared_ptr<TheEndPortalBlockEntity> portal = boost::static_pointer_cast<TheEndPortalBlockEntity>(blockEntity);

    GlStateManager::disableLighting();

    RANDOM.setSeed(31100LL);

    GlStateManager::getMatrix(0, MODELVIEW);
    GlStateManager::getMatrix(1, PROJECTION);

    int passes = getPasses(x * x + y * y + z * z);
    double top = y + getOffset();

    bool fogColorReset = false;

    for (int i = 0; i < passes; ++i)
    {
        GlStateManager::pushMatrix();

        float brightness = 2.0F / (float)(18 - i);

        if (i == 0)
        {
            bindTexture(&END_SKY_LOCATION);
            brightness = 0.15F;
            GlStateManager::enableBlend();
            GlStateManager::blendFunc(4, 5);
        }

        else
        {
            bindTexture(&END_PORTAL_LOCATION);
            fogColorReset = true;
            Minecraft::GetInstance()->mGameRenderer->resetFogColor(true);

            if (i == 1)
            {
                GlStateManager::enableBlend();
                GlStateManager::blendFunc(1, 1);
            }
        }

        GlStateManager::texGenMode(GlStateManager::S, 1);
        GlStateManager::texGenMode(GlStateManager::T, 1);
        GlStateManager::texGenMode(GlStateManager::R, 1);

        GlStateManager::texGenParam(GlStateManager::S, 1, getBuffer(1.0F, 0.0F, 0.0F, 0.0F));
        GlStateManager::texGenParam(GlStateManager::T, 1, getBuffer(0.0F, 1.0F, 0.0F, 0.0F));
        GlStateManager::texGenParam(GlStateManager::R, 1, getBuffer(0.0F, 0.0F, 1.0F, 0.0F));
        GlStateManager::texGenParam(GlStateManager::Q, 0, getBuffer(0.0F, 0.0F, 0.0F, 1.0F));

        GlStateManager::enableTexGen(GlStateManager::S);
        GlStateManager::enableTexGen(GlStateManager::T);
        GlStateManager::enableTexGen(GlStateManager::R);

        GlStateManager::popMatrix();

        GlStateManager::matrixMode(2);
        GlStateManager::pushMatrix();
        GlStateManager::loadIdentity();
        GlStateManager::translatef(0.5F, 0.5F, 0.0F);
        GlStateManager::scalef(0.5F, 0.5F, 1.0F);

        float layer = (float)(i + 1);

        GlStateManager::translatef(17.0F / layer, (layer / 1.5F + 2.0F) * ((float)(System::processTimeInMilliSecs() % 800000LL) / 800000.0F), 0.0F);
        GlStateManager::rotatef((layer * layer * 4321.0F + layer * 9.0F) * 2.0F, 0.0F, 0.0F, 1.0F);

        float scale = 4.5F - layer / 4.0F;

        GlStateManager::scalef(scale, scale, 1.0F);
        GlStateManager::multMatrix(PROJECTION);
        GlStateManager::multMatrix(MODELVIEW);

        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

        builder->useProjectedTexture(true);
        builder->begin();

        float red = RANDOM.nextFloat() * 0.5F + 0.1F;
        float green = RANDOM.nextFloat() * 0.5F + 0.4F;
        float blue = RANDOM.nextFloat() * 0.5F + 0.5F;

        if (i == 0)
        {
            red = 1.0F;
            green = 1.0F;
            blue = 1.0F;
        }

        builder->color(red * brightness, green * brightness, blue * brightness, 1.0F);

        if (portal->shouldRenderFace(Direction::SOUTH))
        {
            builder->vertexDouble(x, y, z + 1.0);
            builder->vertexDouble(x + 1.0, y, z + 1.0);
            builder->vertexDouble(x + 1.0, y + 1.0, z + 1.0);
            builder->vertexDouble(x, y + 1.0, z + 1.0);
        }

        if (portal->shouldRenderFace(Direction::NORTH))
        {
            builder->vertexDouble(x, y + 1.0, z);
            builder->vertexDouble(x + 1.0, y + 1.0, z);
            builder->vertexDouble(x + 1.0, y, z);
            builder->vertexDouble(x, y, z);
        }

        if (portal->shouldRenderFace(Direction::EAST))
        {
            builder->vertexDouble(x + 1.0, y + 1.0, z);
            builder->vertexDouble(x + 1.0, y + 1.0, z + 1.0);
            builder->vertexDouble(x + 1.0, y, z + 1.0);
            builder->vertexDouble(x + 1.0, y, z);
        }

        if (portal->shouldRenderFace(Direction::WEST))
        {
            builder->vertexDouble(x, y, z);
            builder->vertexDouble(x, y, z + 1.0);
            builder->vertexDouble(x, y + 1.0, z + 1.0);
            builder->vertexDouble(x, y + 1.0, z);
        }

        if (portal->shouldRenderFace(Direction::DOWN))
        {
            builder->vertexDouble(x, y, z);
            builder->vertexDouble(x + 1.0, y, z);
            builder->vertexDouble(x + 1.0, y, z + 1.0);
            builder->vertexDouble(x, y, z + 1.0);
        }

        if (portal->shouldRenderFace(Direction::UP))
        {
            builder->vertexDouble(x, top, z + 1.0);
            builder->vertexDouble(x + 1.0, top, z + 1.0);
            builder->vertexDouble(x + 1.0, top, z);
            builder->vertexDouble(x, top, z);
        }

        builder->end();
        builder->useProjectedTexture(false);

        GlStateManager::popMatrix();
        GlStateManager::matrixMode(0);
    }

    GlStateManager::disableBlend();

    GlStateManager::disableTexGen(GlStateManager::S);
    GlStateManager::disableTexGen(GlStateManager::T);
    GlStateManager::disableTexGen(GlStateManager::R);

    GlStateManager::enableLighting();

    if (fogColorReset)
    {
        Minecraft::GetInstance()->mGameRenderer->resetFogColor(false);
    }
}

int TheEndPortalRenderer::getPasses(double distanceSqr)
{
    if (distanceSqr > 36864.0)
    {
        return 1;
    }

    if (distanceSqr > 25600.0)
    {
        return 3;
    }

    if (distanceSqr > 16384.0)
    {
        return 5;
    }

    if (distanceSqr > 9216.0)
    {
        return 7;
    }

    if (distanceSqr > 4096.0)
    {
        return 9;
    }

    if (distanceSqr > 1024.0)
    {
        return 11;
    }

    if (distanceSqr > 576.0)
    {
        return 13;
    }

    if (distanceSqr > 256.0)
    {
        return 14;
    }

    return 15;
}

double TheEndPortalRenderer::getOffset()
{
    return 0.75F;
}
