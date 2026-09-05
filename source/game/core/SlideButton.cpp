#include "game/core/SlideButton.hpp"

#include "game/core/GlStateManager.hpp"
#include "game/core/GuiComponent.hpp"
#include "game/core/Minecraft.hpp"

SlideButton::SlideButton(int id, int x, int y, const Options::Option* option,
                         const std::wstring& message, float value)
    : Button(id, x, y, 150, 20, message),
      mValue(value),
      mSliding(false),
      mOption(option)
{
}

int SlideButton::getYImage(bool hovered)
{
    return 0;
}

void SlideButton::renderBg(Minecraft* minecraft, int mouseX, int mouseY)
{
    if (!mVisible)
    {
        return;
    }

    if (mSliding)
    {
        float value = static_cast<float>(mouseX - (mX + 4)) /
                      static_cast<float>(mWidth - 8);
        if (value < 0.0f || value > 1.0f)
        {
            mValue = value < 0.0f ? 0.0f : 1.0f;
        }
        else
        {
            mValue = value;
        }

        minecraft->mOptions->set(mOption, mValue);
        mMessage.assign(minecraft->mOptions->getMessage(mOption), 0,
                        static_cast<std::wstring::size_type>(-1));
    }

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    blit(mX + static_cast<int>(mValue * static_cast<float>(mWidth - 8)), mY,
         0, 66, 4, 20);
    blit(mX + static_cast<int>(mValue * static_cast<float>(mWidth - 8)) + 4, mY,
         196, 66, 4, 20);
}

bool SlideButton::clicked(Minecraft* minecraft, int mouseX, int mouseY)
{
    if (!Button::clicked(minecraft, mouseX, mouseY))
    {
        return false;
    }

    float value = static_cast<float>(mouseX - (mX + 4)) /
                  static_cast<float>(mWidth - 8);
    if (value < 0.0f || value > 1.0f)
    {
        mValue = value < 0.0f ? 0.0f : 1.0f;
    }
    else
    {
        mValue = value;
    }

    minecraft->mOptions->set(mOption, mValue);
    mMessage.assign(minecraft->mOptions->getMessage(mOption), 0,
                    static_cast<std::wstring::size_type>(-1));
    mSliding = true;
    return true;
}

void SlideButton::released(int mouseX, int mouseY)
{
    mSliding = false;
}
