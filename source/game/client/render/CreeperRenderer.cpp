#include "game/client/render/CreeperRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/CreeperModel.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/Creeper.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/CreeperPowerLayer.hpp"

static ResourceLocation CREEPER_LOCATION(TN_MOB_CREEPER);

CreeperRenderer::CreeperRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new CreeperModel(ModelDefinition()), 0.5F)
{
    addLayer(new CreeperPowerLayer(this));
}

CreeperRenderer::~CreeperRenderer()
{
}

void CreeperRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<Creeper> creeper = boost::static_pointer_cast<Creeper>(entity);
    float swelling = creeper->getSwelling(partialTicks);
    float oscillation = 1.0F + Mth::sin(swelling * 100.0F) * swelling * 0.01F;

    swelling = Mth::clamp(swelling, 0.0F, 1.0F);
    swelling *= swelling;
    swelling *= swelling;

    float horizontalScale = (1.0F + swelling * 0.4F) * oscillation;
    float verticalScale = (1.0F + swelling * 0.1F) / oscillation;
    GlStateManager::scalef(horizontalScale, verticalScale, horizontalScale);
}

int CreeperRenderer::getOverlayColor(boost::shared_ptr<LivingEntity>& entity,
                                     float, float partialTicks)
{
    boost::shared_ptr<Creeper> creeper = boost::static_pointer_cast<Creeper>(entity);
    float swelling = creeper->getSwelling(partialTicks);
    int flash = (int)(swelling * 10.0F);
    if ((flash & 1) == 0)
        return 0;

    int alpha = Mth::clamp((int)(swelling * 0.2F * 255.0F), 0, 255);
    return (int)((unsigned)(255 - alpha) << 24) | 0x30FFFFFF;
}

const ResourceLocation* CreeperRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &CREEPER_LOCATION;
}

Model* CreeperRenderer::createModel(const ModelDefinition& definition)
{
    return new CreeperModel(definition);
}
