#include "game/ui/ConfirmScreen.hpp"

#include "game/core/GuiComponent.hpp"
#include "game/core/SmallButton.hpp"

static int s_confirmScreenTicks = 0;

ConfirmScreen::ConfirmScreen(Screen* parent, const std::wstring& title, const std::wstring& message,
    const std::wstring& yesMessage, const std::wstring& noMessage, int id)
    : Screen(), mParent(parent), mTitle(title), mMessage(message), mYesMessage(yesMessage),
      mNoMessage(noMessage), mId(id)
{
}

ConfirmScreen::~ConfirmScreen()
{
}

void ConfirmScreen::init()
{
    mButtons.push_back(new SmallButton(0, mWidth / 2 - 155, mHeight / 6 + 96, mYesMessage));
    mButtons.push_back(new SmallButton(1, mWidth / 2 + 5, mHeight / 6 + 96, mNoMessage));
}

void ConfirmScreen::buttonClicked(Button* button)
{
    mParent->confirmResult(button->mId == 0, mId);
}

void ConfirmScreen::render(int x, int y, float partialTick)
{
    renderBackground();
    drawCenteredString(mFont, mTitle, mWidth / 2, 70, 0xffffff);
    drawCenteredString(mFont, mMessage, mWidth / 2, 90, 0xffffff);
    Screen::render(x, y, partialTick);

    if (s_confirmScreenTicks++ == 100)
    {
        s_confirmScreenTicks = 0;
        buttonClicked(mButtons[0]);
    }
}
