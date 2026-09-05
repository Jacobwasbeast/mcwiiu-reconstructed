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

static ResourceLocation BEAM_LOCATION;

BeaconRenderer::~BeaconRenderer()
{
}

void BeaconRenderer::renderBeaconBeam(double x, double y, double z, double partialTicks,
                                      double beamHeight, double gameTime, int startY, int endY,
                                      arrayWithLength<const float> colors)
{
    renderBeaconBeam(x, y, z, partialTicks, beamHeight, gameTime, startY, endY, colors,
                     0.15, 0.175);
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
    double offset = Mth::frac(-time * 0.2 - (double)Mth::floor(-time * 0.1));
    double angle = time * 0.025 * -1.5;
    double corner0 = angle + 2.35619455575943;
    double corner1 = angle + 0.7853981852531433;
    double corner2 = angle + 3.9269909262657166;
    double corner3 = angle + 5.497787296772003;
    double u0 = offset + 2.5;
    double u1 = offset + beamHeight * 2.5;
    float red = colors.data[0];
    float green = colors.data[1];
    float blue = colors.data[2];

    double x0 = x + std::cos(corner0) * innerRadius + 0.5;
    double z0 = z + std::sin(corner0) * innerRadius + 0.5;
    double x1 = x + std::cos(corner1) * innerRadius + 0.5;
    double z1 = z + std::sin(corner1) * innerRadius + 0.5;
    double x2 = x + std::cos(corner2) * innerRadius + 0.5;
    double z2 = z + std::sin(corner2) * innerRadius + 0.5;
    double x3 = x + std::cos(corner3) * innerRadius + 0.5;
    double z3 = z + std::sin(corner3) * innerRadius + 0.5;
    double y0 = y + (double)startY;
    double y1 = y + (double)endY;

    builder->begin();
    builder->color(red, green, blue, 0.8F);
    builder->vertexUV((float)x0, (float)y1, (float)z0, 0.0F, (float)u1);
    builder->vertexUV((float)x0, (float)y0, (float)z0, 0.0F, (float)u0);
    builder->vertexUV((float)x1, (float)y0, (float)z1, 1.0F, (float)u0);
    builder->vertexUV((float)x1, (float)y1, (float)z1, 1.0F, (float)u1);
    builder->vertexUV((float)x3, (float)y1, (float)z3, 0.0F, (float)u1);
    builder->vertexUV((float)x3, (float)y0, (float)z3, 0.0F, (float)u0);
    builder->vertexUV((float)x2, (float)y0, (float)z2, 1.0F, (float)u0);
    builder->vertexUV((float)x2, (float)y1, (float)z2, 1.0F, (float)u1);
    builder->vertexUV((float)x1, (float)y1, (float)z1, 0.0F, (float)u1);
    builder->vertexUV((float)x1, (float)y0, (float)z1, 0.0F, (float)u0);
    builder->vertexUV((float)x3, (float)y0, (float)z3, 1.0F, (float)u0);
    builder->vertexUV((float)x3, (float)y1, (float)z3, 1.0F, (float)u1);
    builder->vertexUV((float)x2, (float)y1, (float)z2, 0.0F, (float)u1);
    builder->vertexUV((float)x2, (float)y0, (float)z2, 0.0F, (float)u0);
    builder->vertexUV((float)x0, (float)y0, (float)z0, 1.0F, (float)u0);
    builder->vertexUV((float)x0, (float)y1, (float)z0, 1.0F, (float)u1);
    builder->end();

    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::depthMask(false);
    builder->begin();
    builder->color(red, green, blue, 0.8F);
    double glow0 = x + 0.5 - outerRadius;
    double glow1 = x + 0.5 + outerRadius;
    double glowZ0 = z + 0.5 - outerRadius;
    double glowZ1 = z + 0.5 + outerRadius;
    builder->vertexUV((float)glow0, (float)y1, (float)glowZ0, 0.0F, (float)(u1 + offset));
    builder->vertexUV((float)glow0, (float)y0, (float)glowZ0, 0.0F, (float)offset);
    builder->vertexUV((float)glow1, (float)y0, (float)glowZ0, 1.0F, (float)offset);
    builder->vertexUV((float)glow1, (float)y1, (float)glowZ0, 1.0F, (float)(u1 + offset));
    builder->vertexUV((float)glow1, (float)y1, (float)glowZ1, 0.0F, (float)(u1 + offset));
    builder->vertexUV((float)glow1, (float)y0, (float)glowZ1, 0.0F, (float)offset);
    builder->vertexUV((float)glow0, (float)y0, (float)glowZ1, 1.0F, (float)offset);
    builder->vertexUV((float)glow0, (float)y1, (float)glowZ1, 1.0F, (float)(u1 + offset));
    builder->vertexUV((float)glow1, (float)y1, (float)glowZ0, 0.0F, (float)(u1 + offset));
    builder->vertexUV((float)glow1, (float)y0, (float)glowZ0, 0.0F, (float)offset);
    builder->vertexUV((float)glow1, (float)y0, (float)glowZ1, 1.0F, (float)offset);
    builder->vertexUV((float)glow1, (float)y1, (float)glowZ1, 1.0F, (float)(u1 + offset));
    builder->vertexUV((float)glow0, (float)y1, (float)glowZ1, 0.0F, (float)(u1 + offset));
    builder->vertexUV((float)glow0, (float)y0, (float)glowZ1, 0.0F, (float)offset);
    builder->vertexUV((float)glow0, (float)y0, (float)glowZ0, 1.0F, (float)offset);
    builder->vertexUV((float)glow0, (float)y1, (float)glowZ0, 1.0F, (float)(u1 + offset));
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
            colors.data = &section.mRed;
            colors.length = 3;
            int sectionHeight = section.getHeight();
            renderBeaconBeam(x, y, z, partialTicks, beamHeight, gameTime, height,
                             sectionHeight, colors, 0.15, 0.175);
            height += sectionHeight;
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
    double scale = beacon->getAndUpdateClientSideScale();
    std::vector<BeaconBlockEntity::BeaconBeamSection>* sections = beacon->getBeamSections();
    renderBeaconBeam(x, y, z, a, scale, sections,
                     (double)beacon->getLevel()->getGameTime());
}

void BeaconRenderer::renderSpecialBeamDirection(boost::shared_ptr<BlockEntity> blockEntity, double x,
                                                double y, double z, double xOffset, double yOffset,
                                                double zOffset, float a,
                                                int breakProgress, bool bTranslucent, float alpha,
                                                bool bUseDisplayList)
{
    boost::shared_ptr<BeaconBlockEntity> beacon = boost::static_pointer_cast<BeaconBlockEntity>(blockEntity);
    int direction = beacon->getBeamDirection();
    if (direction < 3)
    {
        render(blockEntity, x, y, z, a, breakProgress, bTranslucent, alpha, bUseDisplayList);
        return;
    }

    BlockPos position = beacon->getBlockPos();
    GlStateManager::pushMatrix();
    if (direction == 3)
    {
        GlStateManager::translatef((float)(x + yOffset - position.x + 0.5),
                                    (float)(y + position.y - xOffset + 1.0),
                                    (float)(z + zOffset - position.z));
        GlStateManager::rotatef(-90.0F, 0.0F, 0.0F, 1.0F);
    }
    else if (direction < 5)
    {
        GlStateManager::translatef((float)(x + position.x - yOffset + 0.5),
                                    (float)(y + xOffset - position.y),
                                    (float)(z + zOffset - position.z));
        GlStateManager::rotatef(90.0F, 0.0F, 0.0F, 1.0F);
    }
    else if (direction == 5)
    {
        GlStateManager::translatef((float)(x + xOffset - position.y),
                                    (float)(y + position.z - zOffset + 1.0),
                                    (float)(z + yOffset - position.x + 0.5));
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
    }
    else
    {
        GlStateManager::translatef((float)(x + xOffset - position.y),
                                    (float)(y + zOffset - position.z),
                                    (float)(z + position.x - yOffset + 0.5));
        GlStateManager::rotatef(-90.0F, 1.0F, 0.0F, 0.0F);
    }
    double scale = beacon->getAndUpdateClientSideScale();
    std::vector<BeaconBlockEntity::BeaconBeamSection>* sections = beacon->getBeamSections();
    renderBeaconBeam(x, y, z, a, scale, sections,
                     (double)beacon->getLevel()->getGameTime());
    GlStateManager::popMatrix();
}

bool BeaconRenderer::shouldRenderOffScreen(boost::shared_ptr<BlockEntity> blockEntity)
{
    return true;
}
