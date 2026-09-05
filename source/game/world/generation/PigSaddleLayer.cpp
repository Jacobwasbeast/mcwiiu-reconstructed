#include "game/world/generation/PigSaddleLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/PigModel.hpp"
#include "game/client/render/PigRenderer.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

class Pig;

static ResourceLocation SADDLE_LOCATION;

PigSaddleLayer::PigSaddleLayer(PigRenderer* renderer)
{
    mRenderer = renderer;
    ModelDefinition modelDefinition;
    mModel = new PigModel(0.5f, modelDefinition);
}

PigSaddleLayer::~PigSaddleLayer()
{
}

const ResourceLocation* PigSaddleLayer::getSaddleTextureLocation() const
{
    return &SADDLE_LOCATION;
}

void PigSaddleLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount, float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool bUseDisplayList)
{
    boost::shared_ptr<Pig> pig = boost::dynamic_pointer_cast<Pig>(entity);

    if (pig->hasSaddle())
    {
        mRenderer->bindTexture(getSaddleTextureLocation());
        mModel->copyPropertiesFrom(mRenderer->getModel());

        boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
        mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, bUseDisplayList);
    }
}

bool PigSaddleLayer::colorsOnDamage()
{
    return false;
}
