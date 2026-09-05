#include "game/client/render/CreeperRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/CreeperModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation CREEPER_LOCATION;

CreeperRenderer::CreeperRenderer(EntityRenderDispatcher* dispatcher)
    : CreeperModel(ModelDefinition())
{
}

CreeperRenderer::~CreeperRenderer()
{
}

void CreeperRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<Creeper> creeper = boost::static_pointer_cast<Creeper>(entity);
    float swelling = creeper->getSwelling(partialTicks);
    float scale = 1.0F + Mth::sin(swelling * 100.0F) * swelling * 0.01F;

    swelling = Mth::clamp(swelling, 0.0F, 1.0F);
    swelling *= swelling;
    swelling *= swelling;

    GlStateManager::scalef((1.0F + swelling * 0.4F) * scale,
                           (1.0F + swelling * 0.1F) / scale,
                           (1.0F + swelling * 0.4F) * scale);
}

unsigned int CreeperRenderer::getOverlayColor(boost::shared_ptr<LivingEntity>& entity, float, float partialTicks)
{
    boost::shared_ptr<Creeper> creeper = boost::static_pointer_cast<Creeper>(entity);
    float swelling = creeper->getSwelling(partialTicks);
    int overlay = (int)(swelling * 10.0F);
    if ((overlay & 1) == 0)
        return 0;

    overlay = Mth::clamp((int)(swelling * 0.2F * 255.0F), 0, 255);
    return (255 - overlay) * 0x1000000 | 0x30FFFFFF;
}

const ResourceLocation* CreeperRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &CREEPER_LOCATION;
}

Model* CreeperRenderer::createModel(const ModelDefinition& definition)
{
    return new CreeperModel(definition);
}
