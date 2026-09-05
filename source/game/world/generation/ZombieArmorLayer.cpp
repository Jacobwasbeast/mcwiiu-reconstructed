#include "game/world/generation/ZombieArmorLayer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ZombieModel.hpp"

ZombieArmorLayer::ZombieArmorLayer(LivingEntityRenderer* renderer, unsigned long long entityType)
    : HumanoidArmorLayer(renderer)
    , mEntityType(entityType)
{
    createArmorModels();
}

ZombieArmorLayer::~ZombieArmorLayer()
{
}

void ZombieArmorLayer::createArmorModels()
{
    if (mModelArmorInner != NULL)
    {
        static_cast<HumanoidModel*>(mModelArmorInner)->mHead->compile(3.0f);
    }

    if (mModelArmor != NULL)
    {
        static_cast<HumanoidModel*>(mModelArmor)->mHead->compile(3.0f);
    }

    ModelDefinition innerDefinition;
    ModelDefinition outerDefinition;

    if (mEntityType == 0x5d550044ULL)
    {
        mModelArmorInner = new ZombieModel(0.75f, false, innerDefinition);
        mModelArmor = new ZombieModel(1.25f, false, outerDefinition);
    }
    else
    {
        mModelArmorInner = new ZombieModel(0.5f, false, innerDefinition);
        mModelArmor = new ZombieModel(1.0f, false, outerDefinition);
    }
}
