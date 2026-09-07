#include "game/world/generation/SheepFurLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/SheepFurModel.hpp"
#include "game/client/render/SheepRenderer.hpp"
#include "game/core/Sheep.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation SHEEP_FUR_LOCATION;

SheepFurLayer::SheepFurLayer(SheepRenderer* renderer)
    : mRenderer(renderer)
{
}

SheepFurLayer::~SheepFurLayer()
{
}

void SheepFurLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                           float limbSwingAmount, float partialTicks, float ageInTicks,
                           float netHeadYaw, float headPitch, float scale,
                           bool bUseDisplayList)
{
    boost::shared_ptr<Sheep> sheep = boost::dynamic_pointer_cast<Sheep>(entity);

    if (!sheep->isSheared() && !sheep->isInvisible())
    {
        mRenderer->bindTexture(&SHEEP_FUR_LOCATION);
        mRenderer->setColourState(sheep, partialTicks);
        mRenderer->mFurModel->copyPropertiesFrom(mRenderer->getModel());

        boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
        mRenderer->mFurModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks,
                                     netHeadYaw, headPitch, scale, bUseDisplayList);
    }
}

bool SheepFurLayer::colorsOnDamage()
{
    return true;
}
