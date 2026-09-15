#include "game/ui/UIControl_EnchantmentBook.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/client/render/BookModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EnchantmentMenu.hpp"
#include "game/ui/UIScene_EnchantingMenu.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/client/render/Textures.hpp"

UIControl_EnchantmentBook::UIControl_EnchantmentBook()
{
    m_controlId = 5;
    mBookModel = NULL;
    mBookTick = 0;
    mBookOpen = 0.0F;
    mBookOpenPrev = 0.0F;
    mBookOpenTarget = 0.0F;
    mBookOpenSpeed = 0.0F;
    mBookRotation = 0.0F;
    mBookRotationPrev = 0.0F;
    mItemInstance = ItemInstance::EMPTY;
}

UIControl_EnchantmentBook::~UIControl_EnchantmentBook()
{
}

void UIControl_EnchantmentBook::tickBook()
{
    UIScene_EnchantingMenu* enchantingMenu = dynamic_cast<UIScene_EnchantingMenu*>(m_scene);
    not_null_ptr<ItemInstance> itemInstance;

    if (enchantingMenu == NULL)
    {
        itemInstance = not_null_ptr<ItemInstance>(new ItemInstance(Items::bow, 1, 0));
    }
    else
    {
        itemInstance = enchantingMenu->getMenu()->getSlot(0)->getItem();
    }

    if (!ItemInstance::matches(itemInstance, mItemInstance, true))
    {
        mItemInstance = itemInstance;
        float target = mBookOpenTarget;
        do
        {
            int r1 = mRandom.nextInt(4);
            int r2 = mRandom.nextInt(4);
            target = target + static_cast<float>(r1 - r2);
            mBookOpenTarget = target;
            if (target + 1.0F < mBookOpen)
            {
                break;
            }
        } while (target - 1.0F <= mBookOpen);
    }
    mBookOpenPrev = mBookOpen;
    mBookRotationPrev = mBookRotation;
    ++mBookTick;

    mBookRotation += 0.2F;

    if (mBookRotation < 0.0F)
    {
        mBookRotation = 0.0F;
    }
    if (mBookRotation > 1.0F)
    {
        mBookRotation = 1.0F;
    }

    float openSpeed = (mBookOpenTarget - mBookOpen) * 0.4F;
    if (openSpeed < -0.2F)
    {
        openSpeed = -0.2F;
    }
    if (openSpeed > 0.2F)
    {
        openSpeed = 0.2F;
    }
    mBookOpenSpeed = (openSpeed - mBookOpenSpeed) * 0.9F + mBookOpenSpeed;
    mBookOpen = mBookOpen + mBookOpenSpeed;
}

void UIControl_EnchantmentBook::render(fuiRect* rect)
{
    const float partialTicks = 1.0F;
    const float widthScale = (rect->xMax - rect->xMin) / m_width;
    const float heightScale = (rect->yMax - rect->yMin) / m_height;

    GlStateManager::pushMatrix();
    GlStateManager::scalef(widthScale, heightScale, 1.0F);
    GlStateManager::translatef(m_width * 0.5F, m_height * 0.5F, 50.0F);
    GlStateManager::scalef(57.0F / widthScale, 360.0F / widthScale, 135.0F);
    GlStateManager::rotatef(135.0F, 0.0F, 1.0F, 0.0F);
    Lighting::turnOn();
    GlStateManager::rotatef(-135.0F, 0.0F, 1.0F, 0.0F);

    Minecraft* minecraft = Minecraft::GetInstance();
    Textures* textures = minecraft->mTextures;
    textures->bind(textures->loadTexture(73));
    GlStateManager::rotatef(20.0F, 1.0F, 0.0F, 0.0F);

    const float rotation = mBookRotationPrev + (mBookRotation - mBookRotationPrev) * partialTicks;
    const float rotationDelta = 1.0F - rotation;
    GlStateManager::translatef(rotationDelta * 0.2F, rotationDelta * 0.1F, rotationDelta * 0.25F);
    GlStateManager::rotatef(-rotationDelta * 90.0F - 90.0F, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(180.0F, 1.0F, 0.0F, 0.0F);

    float spread = mBookOpenPrev + (mBookOpen - mBookOpenPrev) * partialTicks + 0.25F;
    float spread2 = mBookOpenPrev + (mBookOpen - mBookOpenPrev) * partialTicks + 0.75F;
    spread = (spread - std::floor(spread)) * 1.6F - 0.3F;
    spread2 = (spread2 - std::floor(spread2)) * 1.6F - 0.3F;
    if (spread < 0.0F)
    {
        spread = 0.0F;
    }
    if (spread2 < 0.0F)
    {
        spread2 = 0.0F;
    }
    if (spread > 1.0F)
    {
        spread = 1.0F;
    }
    if (spread2 > 1.0F)
    {
        spread2 = 1.0F;
    }

    GlStateManager::enableCull();
    if (mBookModel == NULL)
    {
        mBookModel = new BookModel(ModelDefinition());
    }
    boost::shared_ptr<Entity> entity;
    mBookModel->render(entity, 854.0F, spread, spread2, rotation, 854.0F, 0.0625F, true);
    GlStateManager::disableCull();
    GlStateManager::popMatrix();
    Lighting::turnOff();
    GlStateManager::disableRescaleNormal();
    tickBook();
}
