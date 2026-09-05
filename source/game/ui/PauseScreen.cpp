#include "game/ui/PauseScreen.hpp"

#include <cmath>
#include <string>

#include "game/core/Button.hpp"
#include "game/core/I18n.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"

class OptionsScreen;
class TitleScreen;

extern OptionsScreen* createOptionsScreen(PauseScreen* parent, void* options);
extern TitleScreen* createTitleScreen();

PauseScreen::~PauseScreen()
{
}

void PauseScreen::init()
{
    mButtonListState = 0;

    mButtons.push_back(new Button(1, mWidth / 2 - 100, mHeight / 4 + 104,
                                  std::wstring(L"Save and quit to title")));

    if (mMinecraft->isClientSide())
    {
        mButtons[0]->mMessage = std::wstring(L"Disconnect");
    }

    mButtons.push_back(new Button(4, mWidth / 2 - 100, mHeight / 4 + 8,
                                  std::wstring(L"Back to game")));
    mButtons.push_back(new Button(0, mWidth / 2 - 100, mHeight / 4 + 80,
                                  std::wstring(L"Options...")));
    mButtons.push_back(new Button(5, mWidth / 2 - 100, mHeight / 4 + 32,
                                  98, 20, I18n::get(L"gui.achievements")));
    mButtons.push_back(new Button(6, mWidth / 2 + 2, mHeight / 4 + 32,
                                  98, 20, I18n::get(L"gui.stats")));
}

void PauseScreen::buttonClicked(Button button)
{
    if (button.mId == 0)
    {
        mMinecraft->setScreen(createOptionsScreen(this, mMinecraft->mOptions));
    }

    if (button.mId == 1)
    {
        if (mMinecraft->isClientSide())
        {
            mMinecraft->mGameMode->disconnect();
        }
        mMinecraft->setLevel(NULL, -1, boost::shared_ptr<Player>(), true, false);
        mMinecraft->setScreen(createTitleScreen());
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
    if (mTickCount < 20)
    {
        float phase = (static_cast<float>(mTickCount % 10) - 176.0f + partialTick) / 10.0f * 3.1415927f;
        int color = static_cast<int>(255.0f * (std::sin(phase * 2.0f) * 0.2f + 0.8f));
        GuiComponent::drawString(mFont, std::wstring(L"Saving level..."), 8, mHeight - 16,
                                  (color << 16) | (color << 8) | color);
    }

    GuiComponent::drawCenteredString(mFont, std::wstring(L"Game menu"), mWidth / 2, 40, 0xffffff);
}
