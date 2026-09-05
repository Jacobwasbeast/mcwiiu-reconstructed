#include "game/client/render/WitherBossRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/WitherBossModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/WitherArmorLayer.hpp"
#include "game/core/WitherBoss.hpp"

static ResourceLocation WITHER_LOCATION;
static ResourceLocation WITHER_INVULNERABLE_LOCATION;

WitherBossRenderer::WitherBossRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new WitherBossModel(256.0f, ModelDefinition()), 1.0f)
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
    {
        return &WITHER_INVULNERABLE_LOCATION;
    }
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

    GlStateManager::scalef((1.0F + swelling * 0.4F) * scale,
                           (1.0F + swelling * 0.1F) / scale,
                           (1.0F + swelling * 0.4F) * scale);
}

unsigned int WitherBossRenderer::getOverlayColor(boost::shared_ptr<LivingEntity>& entity, float u, float v)
{
    boost::shared_ptr<WitherBoss> wither = boost::static_pointer_cast<WitherBoss>(entity);
    if (wither->getInvulnerableTicks() < 1)
    {
        return LivingEntityRenderer::getOverlayColor(entity, u, v);
    }

    float alpha = Mth::clamp(wither->getOverlayAlpha(), 0.0F, 1.0F);
    return ((unsigned int)(alpha * 255.0F) << 24) | 0x00FFFFFF;
}

Model* WitherBossRenderer::createModel(const ModelDefinition& definition)
{
    return new WitherBossModel(256.0f, definition);
}
