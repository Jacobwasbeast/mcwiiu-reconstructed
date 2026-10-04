#include "game/world/generation/SkeletonArmorLayer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/SkeletonModel.hpp"

SkeletonArmorLayer::SkeletonArmorLayer(LivingEntityRenderer* renderer)
    : HumanoidArmorLayer(renderer)
{
    createArmorModels();
}

SkeletonArmorLayer::~SkeletonArmorLayer()
{
}

void SkeletonArmorLayer::createArmorModels()
{
    if (mModelArmorInner != NULL)
    {
        delete mModelArmorInner;
    }

    if (mModelArmor != NULL)
    {
        delete mModelArmor;
    }

    mModelArmorInner = new SkeletonModel(0.5f, true, ModelDefinition());
    mModelArmor = new SkeletonModel(1.0f, true, ModelDefinition());
}
