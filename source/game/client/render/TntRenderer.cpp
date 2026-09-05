#include "game/client/render/TntRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

class PrimedTnt : public Entity
{
public:
    int getLife();
};

static ResourceLocation TNT_LOCATION;

TntRenderer::TntRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
    mShadowRadius = 0.5f;
}

TntRenderer::~TntRenderer()
{
}

void TntRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                         float yaw, float partialTick)
{
    boost::shared_ptr<PrimedTnt> tnt = boost::static_pointer_cast<PrimedTnt>(entity);
    BlockRenderDispatcher* blockRenderer = Minecraft::GetInstance()->getBlockRenderer();

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y + 0.5f, (float)z);

    float scale = 1.0f;
    int life = tnt->getLife();
    if ((float)life - partialTick + 1.0f < 10.0f)
    {
        float swelling = Mth::clamp(1.0f - ((float)life - partialTick + 1.0f) / 10.0f, 0.0f, 1.0f);
        swelling *= swelling;
        swelling *= swelling;
        scale = 1.0f + swelling * 0.3f;
        GlStateManager::scalef(scale, scale, scale);
    }

    life = tnt->getLife();
    float alpha = (1.0f - ((float)life - partialTick + 1.0f) / 100.0f) * 0.8f;
    postRender(entity, x, y, z, yaw, partialTick, false);
    GlStateManager::rotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    blockRenderer->renderSingleBlock(Blocks::tnt->defaultBlockState(), 1.0f, 1.0f, true);

    if (!mSolidRender)
    {
        life = tnt->getLife();
        if ((life / 5 & 1) == 0)
        {
            GlStateManager::disableTexture();
            GlStateManager::disableLighting();
            GlStateManager::enableBlend();
            GlStateManager::blendFunc(4, 6);
            GlStateManager::color4f(1.0f, 1.0f, 1.0f, alpha);
            blockRenderer->setColor(false);
            GlStateManager::polygonOffset(-3.0f, -3.0f);
            GlStateManager::enablePolygonOffset();
            blockRenderer->renderSingleBlock(Blocks::tnt->defaultBlockState(), 1.0f, 1.0f, true);
            GlStateManager::polygonOffset(0.0f, 0.0f);
            GlStateManager::disablePolygonOffset();
            blockRenderer->setColor(true);
            GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
            GlStateManager::disableBlend();
            GlStateManager::enableLighting();
            GlStateManager::enableTexture();
        }

        GlStateManager::popMatrix();
        EntityRenderer::render(entity, x, y, z, yaw, partialTick);
    }
    else
    {
        GlStateManager::enableColorMaterial();
        unsigned int teamColor = getTeamColor(entity);
        GlStateManager::setupSolidRenderingTextureCombine((int)teamColor);
        blockRenderer->renderSingleBlock(Blocks::tnt->defaultBlockState(), 1.0f, 1.0f, true);
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
        GlStateManager::popMatrix();
        EntityRenderer::render(entity, x, y, z, yaw, partialTick);
    }
}

const ResourceLocation* TntRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &TNT_LOCATION;
}
