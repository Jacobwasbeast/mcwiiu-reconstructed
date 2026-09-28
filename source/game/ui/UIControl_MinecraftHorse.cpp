#include "game/ui/UIControl_MinecraftHorse.hpp"

#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"

UIControl_MinecraftHorse::UIControl_MinecraftHorse()
    : mScreenWidth(0.0f), mScreenHeight(0.0f), mScaledWidth(0.0f), mScaledHeight(0.0f)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    m_controlType = UIControl::eUIControlType_MinecraftHorse;
    mScreenWidth = (float)minecraft->mWidth;
    mScreenHeight = (float)minecraft->mHeight;
    ScreenSizeCalculator sizeCalculator(minecraft->mOptions, minecraft->mWidth, minecraft->mHeight, -1);
    mScaledWidth = (float)sizeCalculator.getWidth();
    mScaledHeight = (float)sizeCalculator.getHeight();
}

UIControl_MinecraftHorse::~UIControl_MinecraftHorse()
{
}

void UIControl_MinecraftHorse::render(fuiRect* rect)
{
    float width = rect->xMax - rect->xMin;
    float height = rect->yMax - rect->yMin;
    float aspect = mScreenWidth / mScreenHeight;
    float scale = (width / aspect) * 0.7099999785423279f;

    GlStateManager::enableRescaleNormal();
    GlStateManager::enableColorMaterial();
    GlStateManager::pushMatrix();
    GlStateManager::translatef(width * 0.5f, height - height / 7.5f, 50.0f);
    GlStateManager::scalef(-scale, scale, scale);
    GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
    GlStateManager::rotatef(135.0f, 0.0f, 1.0f, 0.0f);
    Lighting::turnOn();
    GlStateManager::rotatef(-135.0f, 0.0f, 1.0f, 0.0f);

    GlStateManager::popMatrix();
    Lighting::turnOff();
    GlStateManager::disableRescaleNormal();
}
