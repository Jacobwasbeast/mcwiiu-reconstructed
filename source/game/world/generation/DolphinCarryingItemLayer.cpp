#include "game/world/generation/DolphinCarryingItemLayer.hpp"

#include <cmath>

#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"

class Minecraft
{
public:
    static Minecraft* GetInstance();
    ItemRenderer* getItemRenderer();
};

DolphinCarryingItemLayer::DolphinCarryingItemLayer(LivingEntityRenderer* renderer)
    : mLivingEntityRenderer(renderer), mItemRenderer(NULL)
{
    mItemRenderer = Minecraft::GetInstance()->getItemRenderer();
}

DolphinCarryingItemLayer::~DolphinCarryingItemLayer()
{
}

void DolphinCarryingItemLayer::renderItemOnNose(const boost::shared_ptr<LivingEntity>& entity,
                                                const not_null_ptr<ItemInstance>& item)
{
    if (item->isEmpty())
        return;

    Block* block = Block::byItem(item->getItem());
    GlStateManager::pushMatrix();

    bool depthMask = mItemRenderer->isGui3d(item) && block->hasInHandRenderOffset();
    if (depthMask)
        GlStateManager::depthMask(false);

    float rotation = static_cast<float>(std::fabs(entity->mXRot)) / 60.0f;
    if (entity->mXRot >= 0.0f)
        GlStateManager::translatef(0.0f, rotation * 0.8f + 1.0f, rotation * 0.2f - 1.0f);
    else
        GlStateManager::translatef(0.0f, 1.0f - rotation * 0.5f, rotation * 0.5f - 1.0f);

    mItemRenderer->renderStatic(item, entity, 7, 0.0f, false, NULL);

    if (depthMask)
        GlStateManager::depthMask(true);
    GlStateManager::popMatrix();
}

void DolphinCarryingItemLayer::render(boost::shared_ptr<LivingEntity>& entity, float, float, float, float, float,
                                      float, float, bool)
{
    not_null_ptr<ItemInstance> firstItem;
    not_null_ptr<ItemInstance> secondItem;

    if (entity->getMainArm() == HumanoidArm::RIGHT)
    {
        firstItem = entity->getOffhandItem();
        secondItem = entity->getMainHandItem();
    }
    else
    {
        firstItem = entity->getMainHandItem();
        secondItem = entity->getOffhandItem();
    }

    if (firstItem->isEmpty() && secondItem->isEmpty())
        return;

    renderItemOnNose(entity, secondItem);
}

bool DolphinCarryingItemLayer::colorsOnDamage()
{
    return false;
}
