#include "game/world/generation/WitchItemLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/WitchModel.hpp"
#include "game/client/render/WitchRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockRenderDispatcher.hpp"

WitchItemLayer::WitchItemLayer(WitchRenderer* renderer)
    : mWitchRenderer(renderer)
{
}

WitchItemLayer::~WitchItemLayer()
{
}

void WitchItemLayer::render(boost::shared_ptr<LivingEntity>& entity, float, float, float, float, float, float scale, bool)
{
    not_null_ptr<ItemInstance> itemInstance = entity->getMainHandItem();

    if (itemInstance->isEmpty())
        return;

    GlStateManager::color3f(1.0f, 1.0f, 1.0f);
    GlStateManager::pushMatrix();

    if (entity->isUsingItem())
    {
        GlStateManager::translatef(0.0f, 0.625f, 0.0f);
        GlStateManager::rotatef(-20.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
    }

    static_cast<WitchModel*>(mWitchRenderer->getModel())->mWitchNose->translateTo(scale);
    GlStateManager::translatef(0.0f, -0.0625f, 0.53125f);

    Item* item = itemInstance->getItem();
    int itemId = Item::getId(item);

    if (itemId < 256 && Minecraft::GetInstance()->getBlockRenderer()->canRenderGeneric(Block::byItem(item)))
    {
        GlStateManager::translatef(0.0f, 0.0625f, 0.125f);
        GlStateManager::rotatef(30.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(-5.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::scalef(0.625f, -0.625f, 0.625f);
        GlStateManager::rotatef(-15.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(40.0f, 0.0f, 0.0f, 1.0f);
    }
    else if (item == Items::potion)
    {
        GlStateManager::translatef(0.0f, 0.1875f, -0.125f);
        GlStateManager::rotatef(-45.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::scalef(0.625f, -0.625f, 0.625f);
        GlStateManager::rotatef(-100.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(-20.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(-15.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(40.0f, 0.0f, 0.0f, 1.0f);
    }
    else if (!item->isHandEquipped())
    {
        GlStateManager::translatef(0.1875f, 0.1875f, 0.0f);
        GlStateManager::scalef(0.875f, 0.875f, 0.875f);
        GlStateManager::rotatef(-20.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(-60.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(-30.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(-15.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(40.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        if (item->isMirroredArt())
        {
            GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
            GlStateManager::translatef(0.0f, -0.0625f, 0.0f);
        }

        mWitchRenderer->translateWeaponItem();
        GlStateManager::translatef(0.0625f, -0.125f, 0.0f);
        GlStateManager::scalef(0.625f, -0.625f, 0.625f);
        GlStateManager::rotatef(0.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(0.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(-15.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(40.0f, 0.0f, 0.0f, 1.0f);
    }

    Minecraft::GetInstance()->getItemInHandRenderer()->renderItem(entity, itemInstance, 2, false, 0, true);
    GlStateManager::popMatrix();
}

bool WitchItemLayer::colorsOnDamage()
{
    return false;
}
