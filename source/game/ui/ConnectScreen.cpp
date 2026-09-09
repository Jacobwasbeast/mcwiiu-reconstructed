#include "game/ui/ConnectScreen.hpp"

#include <string>

#include "game/core/Button.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/ui/TitleScreen.hpp"

ConnectScreen::ConnectScreen(Minecraft* minecraft, const std::wstring& address, int port)
    : Screen(), mConnection(NULL), mFinished(false)
{
    mMinecraft = minecraft;
    minecraft->setLevel(NULL, -1, boost::shared_ptr<Player>(), true, false);
}

ConnectScreen::~ConnectScreen()
{
}

void ConnectScreen::tick()
{
    if (mConnection != NULL)
        mConnection->tick();
}

void ConnectScreen::keyPressed(wchar_t key, int action)
{
}

void ConnectScreen::init()
{
    mButtons.clear();
    std::wstring label = Language::getInstance()->getElement(L"gui_cancel");
    mButtons.push_back(new Button(0, mWidth / 2 - 100, mHeight / 4 + 132, label));
}

void ConnectScreen::buttonClicked(Button* button)
{
    if (button->mId == 0)
    {
        mFinished = true;
        if (mConnection != NULL)
            mConnection->close();
        mMinecraft->setScreen(new TitleScreen());
    }
}

void ConnectScreen::render(int x, int y, float partialTick)
{
    renderBackground();
    Language* language = Language::getInstance();
    if (mConnection == NULL)
    {
        std::wstring message = language->getElement(L"connect_connecting");
        drawCenteredString(mFont, message, mWidth / 2, mHeight / 2 - 50, 0xffffff);
        drawCenteredString(mFont, L"connect", mWidth / 2, mHeight / 2 - 10, 0xffffff);
    }
    else
    {
        std::wstring message = language->getElement(L"connect_authorizing");
        drawCenteredString(mFont, message, mWidth / 2, mHeight / 2 - 50, 0xffffff);
        drawCenteredString(mFont, mConnection->mRemoteAddress, mWidth / 2, mHeight / 2 - 10, 0xffffff);
    }
    Screen::render(x, y, partialTick);
}
