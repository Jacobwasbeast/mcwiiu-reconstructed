#include "game/ui/DisconnectedScreen.hpp"

#include "game/core/Button.hpp"
#include "game/core/GuiComponent.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/TitleScreen.hpp"

DisconnectedScreen::~DisconnectedScreen()
{
}

void DisconnectedScreen::tick()
{
}

void DisconnectedScreen::keyPressed(char key, int action)
{
}

void DisconnectedScreen::init()
{
    std::wstring key(L"gui.toMenu");
    std::wstring label = Language::getInstance()->getElement(key);
    mButtons.push_back(new Button(0, mWidth / 2 - 100, mHeight / 4 + 132, label));
}

void DisconnectedScreen::buttonClicked(Button* button)
{
    if (button->mId == 0)
    {
        mMinecraft->setScreen(new TitleScreen());
    }
}

void DisconnectedScreen::render(int x, int y, float partialTick)
{
    drawCenteredString(mFont, mTitle, mWidth / 2, mHeight / 2 - 50, 0xffffff);
    drawCenteredString(mFont, mMessage, mWidth / 2, mHeight / 2 - 10, 0xffffff);
    Screen::render(x, y, partialTick);
}
