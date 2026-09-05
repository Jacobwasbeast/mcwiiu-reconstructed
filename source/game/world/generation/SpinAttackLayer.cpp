#include "game/world/generation/SpinAttackLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SpinAttackModel.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation SPIN_ATTACK_LOCATION;

SpinAttackLayer::SpinAttackLayer(LivingEntityRenderer* renderer)
    : mRenderer(renderer), mModel(NULL)
{
    ModelDefinition modelDefinition;
    mModel = new SpinAttackModel(modelDefinition);
}

SpinAttackLayer::~SpinAttackLayer()
{
    if (mModel != NULL)
    {
        delete mModel;
    }
}

void SpinAttackLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                             float limbSwingAmount, float partialTicks, float ageInTicks,
                             float netHeadYaw, float headPitch, float scale,
                             bool bUseDisplayList)
{
    if (!entity->getSharedFlag(4))
    {
        return;
    }

    mRenderer->bindTexture(&SPIN_ATTACK_LOCATION);
    boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
    mModel->renderSpin(renderEntity, ageInTicks, scale, bUseDisplayList);
}

bool SpinAttackLayer::colorsOnDamage()
{
    return false;
}
