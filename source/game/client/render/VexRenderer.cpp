#include "game/client/render/VexRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/core/Vex.hpp"

static ResourceLocation VEX_LOCATION;
static ResourceLocation VEX_CHARGING_LOCATION;

VexRenderer::VexRenderer(EntityRenderDispatcher*)
    : VexModel(ModelDefinition())
{
}

VexRenderer::~VexRenderer()
{
}

const ResourceLocation* VexRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    if (Minecraft::InMiniGame(15, true))
    {
        boost::shared_ptr<Vex> vex = boost::static_pointer_cast<Vex>(entity);
        if (vex->isCharging())
            return &VEX_CHARGING_LOCATION;
    }

    return &VEX_LOCATION;
}

void VexRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                         float yaw, float partialTick)
{
    VexModel::render(entity, (float)x, (float)y, (float)z, yaw, partialTick, 1.0F, false);
}

void VexRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    GlStateManager::scalef(0.4F, 0.4F, 0.4F);
}

Model* VexRenderer::createModel(const ModelDefinition& definition)
{
    return new VexModel(definition);
}
