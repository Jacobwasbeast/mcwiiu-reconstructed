#include "game/client/render/WitherBossRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/WitherBossModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/WitherArmorLayer.hpp"

static ResourceLocation WITHER_LOCATION(TN_MOB_WITHER_WITHER);
static ResourceLocation WITHER_INVULNERABLE_LOCATION(TN_MOB_WITHER_WITHER_INVULNERABLE);

WitherBossRenderer::WitherBossRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new WitherBossModel(0.0F, ModelDefinition()), 1.0F)
{
    addLayer(new WitherArmorLayer(this));
}

WitherBossRenderer::~WitherBossRenderer()
{
}

const ResourceLocation* WitherBossRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<WitherBoss> wither = boost::static_pointer_cast<WitherBoss>(entity);
    int invulnerableTicks = wither->getInvulnerableTicks();

    if (invulnerableTicks > 0 && (invulnerableTicks > 80 || (invulnerableTicks / 5) % 2 != 1))
        return &WITHER_INVULNERABLE_LOCATION;

    return &WITHER_LOCATION;
}

void WitherBossRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<WitherBoss> wither = boost::static_pointer_cast<WitherBoss>(entity);
    float swelling = wither->getSwellAmount(partialTicks);
    float scale = 1.0F + Mth::sin(swelling * 100.0F) * swelling * 0.01F;

    swelling = Mth::clamp(swelling, 0.0F, 1.0F);
    swelling *= swelling;
    swelling *= swelling;

    float horizontalScale = (1.0F + swelling * 0.4F) * scale;
    float verticalScale = (1.0F + swelling * 0.1F) / scale;
    GlStateManager::scalef(horizontalScale * 2.0F, verticalScale * 2.0F,
                           horizontalScale * 2.0F);
}

int WitherBossRenderer::getOverlayColor(boost::shared_ptr<LivingEntity>& entity, float u, float v)
{
    if (boost::static_pointer_cast<WitherBoss>(entity)->getInvulnerableTicks() < 1)
        return LivingEntityRenderer::getOverlayColor(entity, u, v);

    double alpha = boost::static_pointer_cast<WitherBoss>(entity)->getOverlayAlpha();
    if (alpha < 0.0)
        alpha = 0.0;
    else if (alpha > 1.0)
        alpha = 1.0;

    int overlay = (int)(alpha * 255.0F);
    return overlay << 24 | 0x00FFFFFF;
}

Model* WitherBossRenderer::createModel(const ModelDefinition& definition)
{
    return new WitherBossModel(0.0F, definition);
}
