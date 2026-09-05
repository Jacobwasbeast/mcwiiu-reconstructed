#include "game/world/generation/WolfCollarLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/WolfRenderer.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Wolf;

static ResourceLocation WOLF_COLLAR_LOCATION;

WolfCollarLayer::WolfCollarLayer(WolfRenderer* renderer)
    : mRenderer(renderer)
{
}

WolfCollarLayer::~WolfCollarLayer()
{
}

void WolfCollarLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                             float limbSwingAmount, float partialTicks, float ageInTicks,
                             float netHeadYaw, float headPitch, float scale,
                             bool bUseDisplayList)
{
    boost::shared_ptr<Wolf> wolf = boost::dynamic_pointer_cast<Wolf>(entity);

    if (wolf->isTame() && !wolf->isInvisible())
    {
        mRenderer->bindTexture(&WOLF_COLLAR_LOCATION);

        const DyeColor* collarColor = wolf->getCollarColor();
        arrayWithLength<const float> colours = collarColor->getTextureDiffuseColors();
        GlStateManager::color3f(colours.data[0], colours.data[1], colours.data[2]);

        boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
        mRenderer->getModel()->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks,
                                       netHeadYaw, headPitch, scale, bUseDisplayList);
    }
}

bool WolfCollarLayer::colorsOnDamage()
{
    return true;
}
