#include "game/client/render/HumanoidHeadModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"

HumanoidHeadModel::HumanoidHeadModel(const ModelDefinition& modelDefinition)
    : SkeletonHeadModel(0, 0, 64, 64, modelDefinition)
{
    mHead = new ModelPart(this, 32, 0);
    mHead->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, 0.25f);
    mHead->setPos(0.0f, 0.0f, 0.0f);
    mHead->compile(0.0625f);
}

HumanoidHeadModel::~HumanoidHeadModel()
{
    delete mHead;
}

void HumanoidHeadModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks,
                                  float netHeadYaw, float headPitch, float scale,
                                  boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    SkeletonHeadModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw,
                                 headPitch, scale, entity, skinAdjustments);
}

void HumanoidHeadModel::render(boost::shared_ptr<Entity>& entity, float limbSwing,
                               float limbSwingAmount, float ageInTicks, float netHeadYaw,
                               float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale,
              entity, _SkinAdjustments());
    mHead->render(scale, useDisplayList, false);
}
