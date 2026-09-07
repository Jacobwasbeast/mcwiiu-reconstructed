#include "game/ui/DeathScreen.hpp"

#include <string>

#include "game/core/Button.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/Screen.hpp"
#include "game/ui/TitleScreen.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

std::wstring _toString(int value);

static int gDeathScreenRenderTicks = 0;

DeathScreen::~DeathScreen()
{
}

void DeathScreen::init()
{
    mButtons.clear();

    mButtons.push_back(new Button(1, mWidth / 2 - 100, mHeight / 4 + 72,
                                  std::wstring(L"Respawn")));
    mButtons.push_back(new Button(2, mWidth / 2 - 100, mHeight / 4 + 96,
                                  std::wstring(L"Title menu")));

    if (mMinecraft->mLevel == NULL)
    {
        mButtons[0]->mActive = false;
    }
}

void DeathScreen::keyPressed(wchar_t key, int action)
{
}

void DeathScreen::buttonClicked(Button* button)
{
    if (button->mId == 1)
    {
        if (mMinecraft->mPlayer)
        {
            mMinecraft->mPlayer->respawn();
        }
        mMinecraft->setScreen(NULL);
    }

    if (button->mId == 2)
    {
        mMinecraft->setLevel(NULL, -1, boost::shared_ptr<Player>(), true, false);
        mMinecraft->setScreen(new TitleScreen());
    }
}

void DeathScreen::render(int x, int y, float partialTick)
{
    fillGradient(0, 0, mWidth, mHeight, 0x60500000, static_cast<int>(0xa0803030));
    GlStateManager::pushMatrix();
    GlStateManager::scalef(0.8f, 0.8f, 0.8f);
    drawCenteredString(mFont, std::wstring(L"Game over!"), mWidth / 4, 30, 0xffffff);
    GlStateManager::popMatrix();

    std::wstring score = std::wstring(L"Score: &e") + _toString(mMinecraft->mPlayer->getScore());
    drawCenteredString(mFont, score, mWidth / 2, 100, 0xffffff);

    Screen::render(x, y, partialTick);

    if (gDeathScreenRenderTicks++ == 100)
    {
        gDeathScreenRenderTicks = 0;
        buttonClicked(mButtons[0]);
    }
}

bool DeathScreen::isPauseScreen()
{
    return false;
}
