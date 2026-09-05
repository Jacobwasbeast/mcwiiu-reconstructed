#include "game/world/generation/HumanoidArmorLayer.hpp"

#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/EquipmentSlot.hpp"

HumanoidArmorLayer::HumanoidArmorLayer(LivingEntityRenderer* renderer)
    : AbstractArmorLayer(renderer)
{
    createArmorModels();
}

HumanoidArmorLayer::~HumanoidArmorLayer()
{
}

void HumanoidArmorLayer::createArmorModels()
{
    ModelDefinition definition;
    mModelArmorInner = new HumanoidModel(definition, 0.5f, true);
    mModelArmor = new HumanoidModel(definition, 1.0f, true);
}

void HumanoidArmorLayer::setPartVisibility(Model* model, const EquipmentSlot* slot)
{
    HumanoidModel* humanoidModel = static_cast<HumanoidModel*>(model);
    humanoidModel->setAllVisible(false);
    switch (slot->getEIndex())
    {
    case EquipmentSlot::eIndex_Feet:
        humanoidModel->mRightLeg->mShow = true;
        humanoidModel->mLeftLeg->mShow = true;
        break;
    case EquipmentSlot::eIndex_Legs:
        humanoidModel->mBody->mShow = true;
        humanoidModel->mRightLeg->mShow = true;
        humanoidModel->mLeftLeg->mShow = true;
        break;
    case EquipmentSlot::eIndex_Chest:
        humanoidModel->mBody->mShow = true;
        humanoidModel->mRightArm->mShow = true;
        humanoidModel->mLeftArm->mShow = true;
        break;
    case EquipmentSlot::eIndex_Head:
        humanoidModel->mHead->mShow = true;
        humanoidModel->mHat->mShow = true;
        break;
    default:
        break;
    }
    humanoidModel->mArmourSlot = slot;
}

void HumanoidArmorLayer::hideAllArmor(Model* model)
{
    static_cast<HumanoidModel*>(model)->setAllVisible(false);
}
