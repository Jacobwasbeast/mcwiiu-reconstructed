#include "game/client/render/BeaconRenderer.hpp"

#include "game/client/render/BlockEntityRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"

#include <boost/shared_ptr.hpp>
#include <vector>
#include <cmath>

static ResourceLocation BEAM_LOCATION(TN_MISC_BEACON_BEAM);

BeaconRenderer::~BeaconRenderer()
{
}

void BeaconRenderer::renderBeaconBeam(double x, double y, double z, double partialTicks,
                                      double beamHeight, double gameTime, int startY, int endY,
                                      arrayWithLength<const float> colors)
{
    renderBeaconBeam(x, y, z, partialTicks, beamHeight, gameTime, startY, endY, colors,
                     0.2, 0.25);
}

void BeaconRenderer::renderBeaconBeam(double x, double y, double z, double partialTicks,
                                      double beamHeight, double gameTime, int startY, int endY,
                                      arrayWithLength<const float> colors, double innerRadius,
                                      double outerRadius)
{
    GlStateManager::disableLighting();
    GlStateManager::colorMask(true, true, true, false);
    GlStateManager::disableCull();
    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
    GlStateManager::blendFunc(4, 1);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    double time = gameTime + partialTicks;
    double signedTime = endY < 0 ? time : -time;
    double v0 = Mth::frac(signedTime * 0.2 - (double)Mth::floor(signedTime * 0.1)) + -1.0;
    double angle = time * 0.025 * -1.5;
    double corner0 = angle + 2.35619455575943;
    double corner1 = angle + 0.7853981852531433;
    double corner2 = angle + 3.9269909262657166;
    double corner3 = angle + 5.497787296772003;
    double v1 = (double)endY * beamHeight * 2.5 + v0;
    float red = colors.data[0];
    float green = colors.data[1];
    float blue = colors.data[2];

    double x0 = x + (std::cos(corner0) * 0.2 + 0.5);
    double z0 = z + (std::sin(corner0) * 0.2 + 0.5);
    double x1 = x + (std::cos(corner1) * 0.2 + 0.5);
    double z1 = z + (std::sin(corner1) * 0.2 + 0.5);
    double x2 = x + (std::cos(corner2) * 0.2 + 0.5);
    double z2 = z + (std::sin(corner2) * 0.2 + 0.5);
    double x3 = x + (std::cos(corner3) * 0.2 + 0.5);
    double z3 = z + (std::sin(corner3) * 0.2 + 0.5);
    double y0 = y + (double)startY;
    double y1 = y + (double)(startY + endY);

    builder->begin();
    builder->color(red, green, blue, 0.125F);
    builder->vertexUV((float)x0, (float)y1, (float)z0, 1.0F, (float)v1);
    builder->vertexUV((float)x0, (float)y0, (float)z0, 1.0F, (float)v0);
    builder->vertexUV((float)x1, (float)y0, (float)z1, 0.0F, (float)v0);
    builder->vertexUV((float)x1, (float)y1, (float)z1, 0.0F, (float)v1);
    builder->vertexUV((float)x3, (float)y1, (float)z3, 1.0F, (float)v1);
    builder->vertexUV((float)x3, (float)y0, (float)z3, 1.0F, (float)v0);
    builder->vertexUV((float)x2, (float)y0, (float)z2, 0.0F, (float)v0);
    builder->vertexUV((float)x2, (float)y1, (float)z2, 0.0F, (float)v1);
    builder->vertexUV((float)x1, (float)y1, (float)z1, 1.0F, (float)v1);
    builder->vertexUV((float)x1, (float)y0, (float)z1, 1.0F, (float)v0);
    builder->vertexUV((float)x3, (float)y0, (float)z3, 0.0F, (float)v0);
    builder->vertexUV((float)x3, (float)y1, (float)z3, 0.0F, (float)v1);
    builder->vertexUV((float)x2, (float)y1, (float)z2, 1.0F, (float)v1);
    builder->vertexUV((float)x2, (float)y0, (float)z2, 1.0F, (float)v0);
    builder->vertexUV((float)x0, (float)y0, (float)z0, 0.0F, (float)v0);
    builder->vertexUV((float)x0, (float)y1, (float)z0, 0.0F, (float)v1);
    builder->end();

    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::depthMask(false);
    builder->begin();
    builder->color(red, green, blue, 0.125F);
    double glowX0 = x + 0.2;
    double glowX1 = x + 0.8;
    double glowZ0 = z + 0.2;
    double glowZ1 = z + 0.8;
    double glowV1 = (double)endY * beamHeight + v0;
    builder->vertexUV((float)glowX0, (float)y1, (float)glowZ0, 1.0F, (float)glowV1);
    builder->vertexUV((float)glowX0, (float)y0, (float)glowZ0, 1.0F, (float)v0);
    builder->vertexUV((float)glowX1, (float)y0, (float)glowZ0, 0.0F, (float)v0);
    builder->vertexUV((float)glowX1, (float)y1, (float)glowZ0, 0.0F, (float)glowV1);
    builder->vertexUV((float)glowX1, (float)y1, (float)glowZ1, 1.0F, (float)glowV1);
    builder->vertexUV((float)glowX1, (float)y0, (float)glowZ1, 1.0F, (float)v0);
    builder->vertexUV((float)glowX0, (float)y0, (float)glowZ1, 0.0F, (float)v0);
    builder->vertexUV((float)glowX0, (float)y1, (float)glowZ1, 0.0F, (float)glowV1);
    builder->vertexUV((float)glowX1, (float)y1, (float)glowZ0, 1.0F, (float)glowV1);
    builder->vertexUV((float)glowX1, (float)y0, (float)glowZ0, 1.0F, (float)v0);
    builder->vertexUV((float)glowX1, (float)y0, (float)glowZ1, 0.0F, (float)v0);
    builder->vertexUV((float)glowX1, (float)y1, (float)glowZ1, 0.0F, (float)glowV1);
    builder->vertexUV((float)glowX0, (float)y1, (float)glowZ1, 1.0F, (float)glowV1);
    builder->vertexUV((float)glowX0, (float)y0, (float)glowZ1, 1.0F, (float)v0);
    builder->vertexUV((float)glowX0, (float)y0, (float)glowZ0, 0.0F, (float)v0);
    builder->vertexUV((float)glowX0, (float)y1, (float)glowZ0, 0.0F, (float)glowV1);
    builder->end();
    GlStateManager::enableLighting();
    GlStateManager::enableTexture();
    GlStateManager::depthMask(true);
    GlStateManager::colorMask(true, true, true, true);
}

void BeaconRenderer::renderBeaconBeam(double x, double y, double z, double partialTicks,
                                      double beamHeight,
                                      std::vector<BeaconBlockEntity::BeaconBeamSection>* sections,
                                      double gameTime)
{
    bindTexture(&BEAM_LOCATION);
    if (beamHeight > 0.0)
    {
        GlStateManager::disableFog();
        int height = 0;
        unsigned int index = 0;
        while (sections->size() > index)
        {
            BeaconBlockEntity::BeaconBeamSection& section = sections->at(index);
            arrayWithLength<const float> colors;
            colors.data = section.getColor();
            colors.length = 3;
            renderBeaconBeam(x, y, z, partialTicks, beamHeight, gameTime, height,
                             section.getHeight(), colors);
            height += section.getHeight();
            ++index;
        }
        GlStateManager::enableFog();
    }
}

void BeaconRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                            float a, int breakProgress, bool bTranslucent, float alpha,
                            bool bUseDisplayList)
{
    boost::shared_ptr<BeaconBlockEntity> beacon = boost::static_pointer_cast<BeaconBlockEntity>(blockEntity);
    renderBeaconBeam(x, y, z, a, beacon->getAndUpdateClientSideScale(), beacon->getBeamSections(),
                     (double)beacon->getLevel()->getGameTime());
}

void BeaconRenderer::renderSpecialBeamDirection(boost::shared_ptr<BlockEntity> blockEntity, double x,
                                                double y, double z, double xOffset, double yOffset,
                                                double zOffset, float a,
                                                int breakProgress, bool bTranslucent, float alpha,
                                                bool bUseDisplayList)
{
    boost::shared_ptr<BeaconBlockEntity> beacon = boost::static_pointer_cast<BeaconBlockEntity>(blockEntity);
    BlockPos position = beacon->getBlockPos();
    int direction = beacon->getBeamDirection();
    switch (direction)
    {
    case 3:
        GlStateManager::pushMatrix();
        GlStateManager::translatef((float)x + ((float)(yOffset - position.y) + 0.5F),
                                    (float)y + ((float)(position.x - xOffset) + 1.0F),
                                    (float)z + (float)(zOffset - position.z));
        GlStateManager::rotatef(-90.0F, 0.0F, 0.0F, 1.0F);
        break;
    case 4:
        GlStateManager::pushMatrix();
        GlStateManager::translatef((float)x + ((float)(position.y - yOffset) + 0.5F),
                                    (float)y + (float)(xOffset - position.x),
                                    (float)z + (float)(zOffset - position.z));
        GlStateManager::rotatef(90.0F, 0.0F, 0.0F, 1.0F);
        break;
    case 5:
        GlStateManager::pushMatrix();
        GlStateManager::translatef((float)x + (float)(xOffset - position.x),
                                    (float)y + ((float)(position.z - zOffset) + 1.0F),
                                    (float)z + ((float)(yOffset - position.y) + 0.5F));
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
        break;
    case 6:
        GlStateManager::pushMatrix();
        GlStateManager::translatef((float)x + (float)(xOffset - position.x),
                                    (float)y + (float)(zOffset - position.z),
                                    (float)z + ((float)(position.y - yOffset) + 0.5F));
        GlStateManager::rotatef(-90.0F, 1.0F, 0.0F, 0.0F);
        break;
    default:
        renderBeaconBeam(x, y, z, a, beacon->getAndUpdateClientSideScale(), beacon->getBeamSections(),
                         (double)beacon->getLevel()->getGameTime());
        return;
    }
    renderBeaconBeam(x, y, z, a, beacon->getAndUpdateClientSideScale(), beacon->getBeamSections(),
                     (double)beacon->getLevel()->getGameTime());
    GlStateManager::popMatrix();
}

bool BeaconRenderer::shouldRenderOffScreen(boost::shared_ptr<BlockEntity> blockEntity)
{
    return true;
}
