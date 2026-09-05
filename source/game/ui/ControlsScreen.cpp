#include "game/ui/ControlsScreen.hpp"

#include "game/core/Button.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/core/SmallButton.hpp"

ControlsScreen::ControlsScreen(Screen* parent, Options* options)
    : Screen(), mParent(parent), mTitle(L"Controls"), mOptions(options), mSelectedKey(-1)
{
}

ControlsScreen::~ControlsScreen()
{
}

int ControlsScreen::getLeftScreenPosition()
{
    return mWidth / 2 - 155;
}

void ControlsScreen::init()
{
    Language* language = Language::getInstance();
    int left = getLeftScreenPosition();

    for (int key = 0; key < 14; key += 2)
    {
        mButtons.push_back(new SmallButton(key, left, mHeight / 6 + (key / 2) * 24, 70, 20,
            mOptions->getKeyMessage(key)));
        mButtons.push_back(new SmallButton(key + 1, left + 160,
            mHeight / 6 + ((key + 1) / 2) * 24, 70, 20,
            mOptions->getKeyMessage(key + 1)));
    }

    mButtons.push_back(new Button(200, mWidth / 2 - 100, mHeight / 6 + 168,
        language->getElement(L"gui_done")));
    mTitle = language->getElement(L"controls_title");
}

void ControlsScreen::buttonClicked(Button* button)
{
    for (int key = 0; key < 12; key += 4)
    {
        mButtons[key]->mMessage = mOptions->getKeyMessage(key);
        mButtons[key + 1]->mMessage = mOptions->getKeyMessage(key + 1);
        mButtons[key + 2]->mMessage = mOptions->getKeyMessage(key + 2);
        mButtons[key + 3]->mMessage = mOptions->getKeyMessage(key + 3);
    }

    for (int key = 12; key < 14; ++key)
    {
        mButtons[key]->mMessage = mOptions->getKeyMessage(key);
    }

    if (button->mId == 200)
    {
        mMinecraft->setScreen(mParent);
        return;
    }

    mSelectedKey = button->mId;
    button->mMessage = L"> " + mOptions->getKeyMessage(mSelectedKey) + L" <";
}

void ControlsScreen::keyPressed(wchar_t key, int action)
{
    if (mSelectedKey >= 0)
    {
        mOptions->setKey(mSelectedKey, action);
        mButtons[mSelectedKey]->mMessage = mOptions->getKeyMessage(mSelectedKey);
        mSelectedKey = -1;
        return;
    }

    Screen::keyPressed(key, action);
}

void ControlsScreen::render(int x, int y, float partialTick)
{
    renderBackground();
    drawCenteredString(mFont, mTitle, mWidth / 2, 20, 0xffffff);

    int left = getLeftScreenPosition();
    for (int key = 0; key < 14; ++key)
    {
        drawString(mFont, mOptions->getKeyDescription(key), left + (key % 2) * 160 + 76,
            mHeight / 6 + (key / 2) * 24 + 7, 0xffffffff);
    }

    Screen::render(x, y, partialTick);
}
