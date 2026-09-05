#include "game/ui/NameEntryScreen.hpp"

#include <string>

#include "game/core/Button.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/GuiComponent.hpp"

std::wstring trimString(const std::wstring& value);

NameEntryScreen::~NameEntryScreen()
{
}

void NameEntryScreen::init()
{
    mButtons.clear();
    mButtons.push_back(new Button(0, mWidth / 2 - 100, mHeight / 4 + 120,
                                  std::wstring(L"Save")));
    mButtons.push_back(new Button(1, mWidth / 2 - 100, mHeight / 4 + 144,
                                  std::wstring(L"Cancel")));

    mButtons[0]->active = trimString(mName).size() > 1;
}

void NameEntryScreen::removed()
{
}

void NameEntryScreen::tick()
{
    ++mTickCount;
}

void NameEntryScreen::buttonClicked(Button* button)
{
    if (button->active)
    {
        if (button->mId == 0)
        {
            if (trimString(mName).size() > 1)
            {
                mMinecraft->setScreen(mParentScreen);
            }
        }

        if (button->mId == 1)
        {
            mMinecraft->setScreen(mParentScreen);
        }
    }
}

void NameEntryScreen::keyPressed(wchar_t key, int action)
{
    if (action == 29 && !mName.empty())
    {
        mName.assign(mName, 0, mName.size() - 1);
    }

    if (ALLOWED_CHARACTERS.find(&key, 0, 1) != std::wstring::npos
        && mName.size() < 64)
    {
        mName += key;
    }

    mButtons[0]->active = trimString(mName).size() > 1;
}

void NameEntryScreen::render(int x, int y, float partialTick)
{
    renderBackground();
    drawCenteredString(mFont, mTitle, mWidth / 2, 40, 0xffffff);

    int centerX = mWidth / 2;
    int centerY = mHeight / 2;
    fill(centerX - 101, centerY - 11, centerX + 101, centerY + 11, 0xffa0a0a0);
    fill(centerX - 100, centerY - 10, centerX + 100, centerY + 10, 0xff000000);

    const wchar_t* cursor = (mTickCount / 6 & 1) == 0 ? L"_" : L"";
    std::wstring displayedName = mName + cursor;
    drawString(mFont, displayedName, centerX - 96, centerY - 4, 0xe0e0e0);

    Screen::render(x, y, partialTick);
}
