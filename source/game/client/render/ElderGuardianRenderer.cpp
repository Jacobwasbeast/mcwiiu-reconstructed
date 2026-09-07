#include "game/client/render/ElderGuardianRenderer.hpp"

#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation ELDER_GUARDIAN_LOCATION;

ElderGuardianRenderer::ElderGuardianRenderer(EntityRenderDispatcher* dispatcher)
    : GuardianRenderer(dispatcher)
{
}

ElderGuardianRenderer::~ElderGuardianRenderer()
{
}

void ElderGuardianRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    GlStateManager::scalef(2.35F, 2.35F, 2.35F);
}

const ResourceLocation* ElderGuardianRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &ELDER_GUARDIAN_LOCATION;
}
