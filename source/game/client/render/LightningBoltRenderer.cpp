#include "game/client/render/LightningBoltRenderer.hpp"

#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/LightningBolt.hpp"
#include "game/util/Random.hpp"

#include <boost/shared_ptr.hpp>
#include <cstring>

LightningBoltRenderer::LightningBoltRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
}

LightningBoltRenderer::~LightningBoltRenderer()
{
}

void LightningBoltRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                   float yaw, float partialTick)
{
    boost::shared_ptr<LightningBolt> lightningBolt = boost::static_pointer_cast<LightningBolt>(entity);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();

    GlStateManager::disableTexture();
    GlStateManager::disableLighting();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 1);

    Random random(lightningBolt->mSeed);
    double xOffsets[8];
    double zOffsets[8];
    double xOffset = 0.0;
    double zOffset = 0.0;
    int index;
    for (index = 7; index >= 0; --index)
    {
        xOffsets[index] = xOffset;
        zOffsets[index] = zOffset;
        xOffset += (double)(random.nextInt(11) - 5) * 0.1;
        zOffset += (double)(random.nextInt(11) - 5) * 0.1;
    }
    double initialXOffset = (double)(random.nextInt(11) - 5) * 0.1;
    double initialZOffset = (double)(random.nextInt(11) - 5) * 0.1;

    int branch;
    for (branch = 0; branch < 4; ++branch)
    {
        double branchX = x + 0.5;
        double branchZ = z + 0.5;
        double width = 0.1 + (double)branch * 0.2;
        int top = 7;
        int bottom = 0;
        if (branch > 0)
        {
            top = 7 - branch;
            bottom = 5 - branch;
        }

        double previousX = branchX + xOffsets[top] - initialXOffset;
        double previousZ = branchZ + zOffsets[top] - initialZOffset;
        int segment;
        for (segment = top; segment >= bottom; --segment)
        {
            double nextX = previousX;
            double nextZ = previousZ;
            if (branch == 0)
            {
                nextX += (double)(random.nextInt(11) - 5) * 0.1;
                nextZ += (double)(random.nextInt(11) - 5) * 0.1;
            }
            else
            {
                nextX += (double)(random.nextInt(31) - 15) * 0.1;
                nextZ += (double)(random.nextInt(31) - 15) * 0.1;
            }

            builder->begin(1);
            builder->color(0.45F, 0.45F, 0.45F, 0.3F);
            double upperWidth = width * ((double)segment * 0.1 + 1.0);
            double lowerWidth = branch == 0 ? width * ((double)(segment - 1) * 0.1 + 1.0) : width;
            double x0 = x + 0.5 - upperWidth;
            double x1 = x + 0.5 + upperWidth;
            double z0 = z + 0.5 - upperWidth;
            double z1 = z + 0.5 + upperWidth;
            double x2 = x + 0.5 - lowerWidth;
            double x3 = x + 0.5 + lowerWidth;
            double z2 = z + 0.5 - lowerWidth;
            double z3 = z + 0.5 + lowerWidth;
            double y0 = y + (double)segment;
            double y1 = y + (double)(segment + 1);

            builder->vertex((float)(x2 + nextX - x - 0.5), (float)y0,
                            (float)(z2 + nextZ - z - 0.5));
            builder->vertex((float)(x0 + previousX - x - 0.5), (float)y1,
                            (float)(z0 + previousZ - z - 0.5));
            builder->vertex((float)(x3 + nextX - x - 0.5), (float)y0,
                            (float)(z3 + nextZ - z - 0.5));
            builder->vertex((float)(x1 + previousX - x - 0.5), (float)y1,
                            (float)(z1 + previousZ - z - 0.5));
            builder->vertex((float)(x3 + nextX - x - 0.5), (float)y0,
                            (float)(z2 + nextZ - z - 0.5));
            builder->vertex((float)(x1 + previousX - x - 0.5), (float)y1,
                            (float)(z0 + previousZ - z - 0.5));
            builder->vertex((float)(x2 + nextX - x - 0.5), (float)y0,
                            (float)(z3 + nextZ - z - 0.5));
            builder->vertex((float)(x0 + previousX - x - 0.5), (float)y1,
                            (float)(z1 + previousZ - z - 0.5));
            builder->vertex((float)(x2 + nextX - x - 0.5), (float)y0,
                            (float)(z2 + nextZ - z - 0.5));
            builder->vertex((float)(x0 + previousX - x - 0.5), (float)y1,
                            (float)(z0 + previousZ - z - 0.5));
            builder->vertex((float)(x3 + nextX - x - 0.5), (float)y0,
                            (float)(z3 + nextZ - z - 0.5));
            builder->vertex((float)(x1 + previousX - x - 0.5), (float)y1,
                            (float)(z1 + previousZ - z - 0.5));
            builder->vertex((float)(x3 + nextX - x - 0.5), (float)y0,
                            (float)(z2 + nextZ - z - 0.5));
            builder->vertex((float)(x1 + previousX - x - 0.5), (float)y1,
                            (float)(z0 + previousZ - z - 0.5));
            builder->end();

            previousX = nextX;
            previousZ = nextZ;
        }
    }

    GlStateManager::disableBlend();
    GlStateManager::enableLighting();
    GlStateManager::enableTexture();
}
