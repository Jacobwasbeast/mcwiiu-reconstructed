#include "game/client/render/SpectralArrowRenderer.hpp"

#include "game/util/ResourceLocation.hpp"

static ResourceLocation SPECTRAL_ARROW_LOCATION;

SpectralArrowRenderer::SpectralArrowRenderer(EntityRenderDispatcher* dispatcher)
    : ArrowRenderer(dispatcher)
{
}

SpectralArrowRenderer::~SpectralArrowRenderer()
{
}

const ResourceLocation* SpectralArrowRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &SPECTRAL_ARROW_LOCATION;
}
