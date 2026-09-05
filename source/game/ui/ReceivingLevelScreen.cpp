#include "game/ui/ReceivingLevelScreen.hpp"

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/core/Language.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ClientboundKeepAlivePacket.hpp"

ReceivingLevelScreen::~ReceivingLevelScreen()
{
}

void ReceivingLevelScreen::keyPressed(char key, int action)
{
}

void ReceivingLevelScreen::init()
{
    mButtons.clear();
}

void ReceivingLevelScreen::tick()
{
    ++mTickCount;

    if (mTickCount % 20 == 0)
    {
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundKeepAlivePacket()));
    }

    if (mConnection != NULL)
    {
        mConnection->tick();
    }
}

void ReceivingLevelScreen::buttonClicked(Button* button)
{
}

void ReceivingLevelScreen::render(int x, int y, float partialTick)
{
    renderDirtBackground(0);

    std::wstring message = Language::getInstance()->getElement(
        std::wstring(L"multiplayer.downloadingTerrain"));
    drawCenteredString(mFont, message, mWidth / 2, mHeight / 2 - 50, 0xffffff);

    Screen::render(x, y, partialTick);
}
