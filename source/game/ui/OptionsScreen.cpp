#include "game/ui/OptionsScreen.hpp"

#include <string>

#include "game/core/Button.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/core/SmallButton.hpp"
#include "game/core/SlideButton.hpp"
#include "game/ui/VideoSettingsScreen.hpp"
#include "game/ui/ControlsScreen.hpp"

OptionsScreen::OptionsScreen(Screen* parentScreen, Options* options)
{
    mTitle = L"Options";
    mParentScreen = parentScreen;
    mOptions = options;
}

OptionsScreen::~OptionsScreen()
{
}

void OptionsScreen::init()
{
    Language* language = Language::getInstance();
    mTitle = language->getElement(L"options.title");

    const Options::Option* options[5] = {
        Options::Option::music,
        Options::Option::sound,
        Options::Option::invertMouse,
        Options::Option::sensitivity,
        Options::Option::difficulty
    };
    int index;

    for (index = 0; index < 5; ++index)
    {
        const Options::Option* option = options[index];
        if (option->isProgress())
        {
            std::wstring message = mOptions->getMessage(option);
            int id = option->getId();
            double value = mOptions->getProgressValue(option);
            mButtons.push_back(new SlideButton(id, mWidth / 2 - 155 + (index % 2) * 160, mHeight / 6 + (index / 2) * 24, option, message, (float)value));
        }
        else
        {
            std::wstring message = mOptions->getMessage(option);
            int id = option->getId();
            mButtons.push_back(new SmallButton(id, mWidth / 2 - 155 + (index % 2) * 160, mHeight / 6 + (index / 2) * 24, option, message));
        }
    }

    mButtons.push_back(new Button(101, mWidth / 2 - 100, mHeight / 6 + 108,
                                  language->getElement(L"options.video")));
    mButtons.push_back(new Button(100, mWidth / 2 - 100, mHeight / 6 + 132,
                                  language->getElement(L"options.controls")));
    mButtons.push_back(new Button(200, mWidth / 2 - 100, mHeight / 6 + 168,
                                  language->getElement(L"gui.done")));
}

void OptionsScreen::buttonClicked(Button* button)
{
    if (!button->mActive)
        return;

    if (button->mId < 100)
    {
        if (dynamic_cast<SmallButton*>(button) != NULL)
        {
            mOptions->toggle(static_cast<SmallButton*>(button)->getOption(), 1);
            button->mMessage = mOptions->getMessage(Options::Option::getItem(button->mId));
        }
    }

    if (button->mId == 101)
    {
        mMinecraft->mOptions->save();
        mMinecraft->setScreen(new VideoSettingsScreen(this, mOptions));
    }
    if (button->mId == 100)
    {
        mMinecraft->mOptions->save();
        mMinecraft->setScreen(new ControlsScreen(this, mOptions));
    }
    if (button->mId == 200)
    {
        mMinecraft->mOptions->save();
        mMinecraft->setScreen(mParentScreen);
    }
}

void OptionsScreen::render(int x, int y, float partialTick)
{
    renderBackground();
    drawCenteredString(mFont, mTitle, mWidth / 2, 20, 0xffffff);
    Screen::render(x, y, partialTick);
}
