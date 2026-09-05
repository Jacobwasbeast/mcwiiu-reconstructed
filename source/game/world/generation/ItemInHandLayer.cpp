#include "game/world/generation/ItemInHandLayer.hpp"

#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/HumanoidModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/item/ItemInstance.hpp"

class LivingEntity
{
public:
    virtual const HumanoidArm* getMainArm();
    virtual not_null_ptr<ItemInstance> getMainHandItem();
    virtual not_null_ptr<ItemInstance> getOffhandItem();
    virtual bool isFallFlying();
};

class ItemInHandRenderer
{
public:
    void renderItem(boost::shared_ptr<LivingEntity> entity, not_null_ptr<ItemInstance> item,
                    ItemTransforms::TransformType transformType, bool leftHand, int seed, bool blit);
};

class Minecraft
{
public:
    static Minecraft* GetInstance();
    ItemInHandRenderer* getItemInHandRenderer();
};

ItemInHandLayer::ItemInHandLayer(LivingEntityRenderer* renderer)
    : mLivingEntityRenderer(renderer)
{
}

ItemInHandLayer::~ItemInHandLayer()
{
}

void ItemInHandLayer::render(boost::shared_ptr<LivingEntity>& entity, float, float, float, float, float, float,
                             float, bool)
{
    not_null_ptr<ItemInstance> rightHandItem;
    not_null_ptr<ItemInstance> leftHandItem;

    if (entity->getMainArm() == HumanoidArm::RIGHT)
    {
        rightHandItem = entity->getMainHandItem();
        leftHandItem = entity->getOffhandItem();
    }
    else
    {
        rightHandItem = entity->getOffhandItem();
        leftHandItem = entity->getMainHandItem();
    }

    if (rightHandItem->isEmpty() && leftHandItem->isEmpty())
        return;

    GlStateManager::pushMatrix();

    if (static_cast<HumanoidModel*>(mLivingEntityRenderer->getModel(entity))->mCrouching)
    {
        GlStateManager::translatef(0.0f, 0.75f, 0.0f);
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
    }

    renderArmWithItem(entity, rightHandItem, ItemTransforms::THIRD_PERSON_RIGHT_HAND, HumanoidArm::RIGHT);
    renderArmWithItem(entity, leftHandItem, ItemTransforms::THIRD_PERSON_LEFT_HAND, HumanoidArm::LEFT);

    GlStateManager::popMatrix();
}

void ItemInHandLayer::renderArmWithItem(boost::shared_ptr<LivingEntity> entity, not_null_ptr<ItemInstance> item,
                                        ItemTransforms::TransformType transformType, const HumanoidArm* arm)
{
    if (item->isEmpty())
        return;

    GlStateManager::pushMatrix();
    mLivingEntityRenderer->translateWeaponItem();

    bool leftHand = arm == HumanoidArm::LEFT;
    if (entity->isFallFlying())
    {
        GlStateManager::translatef(0.0f, -0.175f, 0.0f);
        GlStateManager::rotatef(6.0f, 12.0f, 0.0f, 0.0f);
    }

    GlStateManager::rotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::translatef((leftHand ? -1.0f : 1.0f) * 0.0625f, 0.125f, -0.625f);

    Minecraft::GetInstance()->getItemInHandRenderer()->renderItem(entity, item, transformType, leftHand, 0, true);
    GlStateManager::popMatrix();
}

void ItemInHandLayer::translateToHand(const HumanoidArm* arm)
{
    static_cast<HumanoidModel*>(mLivingEntityRenderer->getModel())->translateToHandItem(0.0625f, arm);
}

bool ItemInHandLayer::colorsOnDamage()
{
    return false;
}
