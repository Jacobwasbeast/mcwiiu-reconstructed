#include "game/ui/PauseScreen.hpp"

#include <cmath>
#include <string>

#include "game/core/Button.hpp"
#include "game/core/I18n.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/ui/OptionsScreen.hpp"
#include "game/ui/TitleScreen.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"

PauseScreen::~PauseScreen()
{
}

void PauseScreen::init()
{
    mButtonListState = 0;
    mButtons.clear();

    mButtons.push_back(new Button(1, mWidth / 2 - 100, mHeight / 4 + 104,
                                  std::wstring(L"Save and quit to title")));

    if (mMinecraft->isClientSide())
    {
        mButtons[0]->mMessage.assign(L"Disconnect");
    }

    mButtons.push_back(new Button(4, mWidth / 2 - 100, mHeight / 4 + 8,
                                  std::wstring(L"LBack to game")));
    mButtons.push_back(new Button(0, mWidth / 2 - 100, mHeight / 4 + 80,
                                  std::wstring(L"LOptions...")));
    mButtons.push_back(new Button(5, mWidth / 2 - 100, mHeight / 4 + 32,
                                  98, 20, I18n::get(L"gui.achievements")));
    mButtons.push_back(new Button(6, mWidth / 2 + 2, mHeight / 4 + 32,
                                  98, 20, I18n::get(L"gui.stats")));
}

void PauseScreen::buttonClicked(Button button)
{
    if (button.mId == 0)
    {
        mMinecraft->setScreen(new OptionsScreen(this, mMinecraft->mOptions));
    }

    if (button.mId == 1)
    {
        if (mMinecraft->isClientSide())
        {
            mMinecraft->mLevel->disconnect(true);
        }
        mMinecraft->setLevel(NULL, -1, boost::shared_ptr<Player>(), true, false);
        mMinecraft->setScreen(new TitleScreen());
    }

    if (button.mId == 4)
    {
        mMinecraft->setScreen(NULL);
    }
}

void PauseScreen::tick()
{
    ++mTickCount;
}

void PauseScreen::render(int x, int y, float partialTick)
{
    renderBackground();

    if (mTickCount < 20)
    {
        float phase = (mTickCount % 10 + partialTick) / 10.0f * 3.1415927f;
        int color = static_cast<int>(255.0f * (Mth::sin(phase * 2.0f) * 0.2f + 0.8f));
        drawString(mFont, std::wstring(L"Saving level.."), 8, mHeight - 16,
                   (color << 16) | (color << 8) | color);
    }

    drawCenteredString(mFont, std::wstring(L"Game menu"), mWidth / 2, 40, 0xffffff);
    Screen::render(x, y, partialTick);
}
