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
        static_cast<HumanoidModel*>(mModelArmorInner)->mHead->compile(3.0f);
    }

    if (mModelArmor != NULL)
    {
        static_cast<HumanoidModel*>(mModelArmor)->mHead->compile(3.0f);
    }

    ModelDefinition definition;
    mModelArmorInner = new SkeletonModel(0.5f, true, definition);
    mModelArmor = new SkeletonModel(1.0f, true, definition);
}
