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
        boost::shared_ptr<Packet> packet(new ClientboundKeepAlivePacket());
        mConnection->send(packet);
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

    {
        Language* language = Language::getInstance();
        std::wstring key(L"multiplayer.downloadingTerrain");
        std::wstring message = language->getElement(key);
        drawCenteredString(mFont, message, mWidth / 2, mHeight / 2 - 50, 0xffffff);
    }

    Screen::render(x, y, partialTick);
}
